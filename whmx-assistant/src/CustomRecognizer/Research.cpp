#include "Research.h"

#include <map>
#include <array>
#include <opencv2/imgproc.hpp>

namespace Rec::Research {

using namespace maa;

enum class GradeFaceCorner {
    Top,
    Left,
    Right,
};

struct GradeOptionPartInfo {
    int     index;
    MaaRect geo;
    cv::Mat image;
};

struct GradeOptionFaceInfo {
    int     index;
    MaaRect origin_geo;
    cv::Mat image;
};

static void grade_face_tilt_correct(const cv::Mat &src, cv::Mat &dst, GradeFaceCorner corner) {
    switch (corner) {
        case GradeFaceCorner::Top: {
            cv::resize(src, dst, cv::Size(src.cols / 2, src.rows));
            const cv::Point2i center{dst.cols / 2, dst.rows / 2};
            const auto        transform = cv::getRotationMatrix2D(center, -45.0, 1.41421);
            cv::warpAffine(dst, dst, transform, dst.size());
        } break;
        case GradeFaceCorner::Left: {
            std::array<cv::Point2f, 3> src_points{
                cv::Point2f(0, 0),
                cv::Point2f(src.cols, src.rows / 3),
                cv::Point2f(0, src.rows / 3 * 2),
            };
            std::array<cv::Point2f, 3> dst_points{
                cv::Point2f(0, 0),
                cv::Point2f(src.cols, 0),
                cv::Point2f(0, src.rows / 3 * 2),
            };
            auto transform = cv::getAffineTransform(src_points.data(), dst_points.data());
            cv::warpAffine(src, dst, transform, src.size());
            dst = dst.rowRange(0, dst.rows / 3 * 2);
        } break;
        case GradeFaceCorner::Right: {
            std::array<cv::Point2f, 3> src_points{
                cv::Point2f(src.cols, 0),
                cv::Point2f(src.cols, src.rows / 3 * 2),
                cv::Point2f(0, src.rows / 3),
            };
            std::array<cv::Point2f, 3> dst_points{
                cv::Point2f(src.cols, 0),
                cv::Point2f(src.cols, src.rows / 3 * 2),
                cv::Point2f(0, 0),
            };
            auto transform = cv::getAffineTransform(src_points.data(), dst_points.data());
            cv::warpAffine(src, dst, transform, src.size());
            dst = dst.rowRange(0, dst.rows / 3 * 2);
        } break;
    }
}

cv::Mat crop_grade_option_face(const GradeOptionPartInfo &part, const GradeOptionFaceInfo &face) {
    const int dx = face.origin_geo.x - part.geo.x;
    const int dy = face.origin_geo.y - part.geo.y;
    const int w  = face.origin_geo.width;
    const int h  = face.origin_geo.height;
    return part.image.rowRange(dy, dy + h).colRange(dx, dx + w);
};

coro::Promise<AnalyzeResult> ParseGradeOptionsOnModify::research__parse_grade_options_on_modify(
    SyncContextHandle context, ImageHandle image, std::string_view task_name, std::string_view param) {
    //! FIXME: the method only works under 1280x720 resolution

    std::array<GradeOptionPartInfo, 2> parts;
    {
        const int width  = 372 - 168;
        const int height = 496 - 260;
        const int top    = 260;
        const int x_0    = 168;
        const int x_1    = 500;

        cv::Mat im(image->height(), image->width(), image->type(), image->raw_data());
        parts[0].index = 0;
        parts[0].geo   = MaaRect{x_0, top, width, height};
        parts[0].image = im.rowRange(top, top + height).colRange(x_0, x_0 + width);
        parts[1].index = 1;
        parts[1].geo   = MaaRect{x_1, top, width, height};
        parts[1].image = im.rowRange(top, top + height).colRange(x_1, x_1 + width);
    }

    std::array<GradeOptionFaceInfo, 6> faces;
    {
        int face_index = 0;
        for (const auto &part : parts) {
            {
                auto &face             = faces[face_index];
                face.index             = face_index++;
                face.origin_geo.x      = part.geo.x;
                face.origin_geo.y      = part.geo.y;
                face.origin_geo.width  = part.geo.width;
                face.origin_geo.height = part.geo.height / 2;
                const auto face_im     = crop_grade_option_face(part, face);
                grade_face_tilt_correct(face_im, face.image, GradeFaceCorner::Top);
            }
            {
                auto &face             = faces[face_index];
                face.index             = face_index++;
                face.origin_geo.x      = part.geo.x;
                face.origin_geo.y      = part.geo.y + part.geo.height / 4;
                face.origin_geo.width  = part.geo.width / 2;
                face.origin_geo.height = part.geo.y + part.geo.height - face.origin_geo.y;
                const auto face_im     = crop_grade_option_face(part, face);
                grade_face_tilt_correct(face_im, face.image, GradeFaceCorner::Left);
            }
            {
                auto &face             = faces[face_index];
                face.index             = face_index++;
                face.origin_geo.x      = part.geo.x + part.geo.width / 2;
                face.origin_geo.y      = part.geo.y + part.geo.height / 4;
                face.origin_geo.width  = part.geo.width / 2;
                face.origin_geo.height = part.geo.y + part.geo.height - face.origin_geo.y;
                const auto face_im     = crop_grade_option_face(part, face);
                grade_face_tilt_correct(face_im, face.image, GradeFaceCorner::Right);
            }
        }
    }

    struct RecTask {
        std::shared_ptr<details::Image> image;
        coro::Promise<AnalyzeResult>    promise;
    };

    const json::object params{
        {"recognition", "OCR"           },
        {"model",       "ppocr_v4/zh_CN"},
    };
    const json::object task_param{
        {"OCR", params}
    };

    std::array<RecTask, 6> tasks;
    for (const auto &[index, geo, image] : faces) {
        auto &[image_object, promise] = tasks[index];
        image_object                  = details::Image::make();
        MaaSetImageRawData(image_object->handle(), image.data, image.cols, image.rows, image.type());
        promise = context->run_recognition(image_object, "OCR", task_param);
    }

    constexpr int UNKNOWN_GRADE = -1;

    static std::map<std::string_view /* utf8 */, int> GRADE_TABLE{
        {"\xe4\xb8\xad", 0},
        {"\xe8\x89\xaf", 1},
        {"\xe4\xbc\x98", 2},
    };

    json::array recog_results;
    for (int i = 0; i < tasks.size(); ++i) {
        int grade = UNKNOWN_GRADE;

        const auto recog_resp = co_await tasks[i].promise;
        const auto face_resp  = json::parse(recog_resp.rec_detail);
        do {
            //! TODO: fallback to "all" if "best" is not found
            if (!face_resp->contains("best")) { break; }
            const auto &best_recog = face_resp->at("best");
            if (!best_recog.contains("text")) { break; }
            const auto &text = best_recog.at("text").as_string();
            if (GRADE_TABLE.count(text)) { grade = GRADE_TABLE[text]; }
        } while (0);

        const auto &geo = faces[i].origin_geo;
        recog_results.push_back(json::object({
            {"index", i                                               },
            {"grade", grade                                           },
            {"box",   json::array{geo.x, geo.y, geo.width, geo.height}},
        }));
    }

    AnalyzeResult resp;
    resp.rec_box    = MaaRect{0, 0, 0, 0};
    resp.rec_detail = recog_results.to_string();
    resp.result     = true;

    co_return resp;
}

} // namespace Rec::Research
