/* Copyright 2024 周上行Ryer

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "FourInRow.h"
#include "../Algorithm.h"

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QDebug>
#include <opencv2/imgproc.hpp>
#include <optional>
#include <QtCore/QElapsedTimer>

using namespace maa;

namespace Action {

class Game {
public:
    constexpr static int ROW        = 6;
    constexpr static int COL        = 7;
    constexpr static int NON_PLAYER = 0;

public:
    using Board = std::array<std::array<int, ROW>, COL>;

    struct EvalResult {
        bool done;
        int  winner;
    };

    struct Move {
        int row;
        int col;

        bool operator<(const Move &other) const {
            return QPair<int, int>(row, col) < QPair<int, int>(other.row, other.col);
        }

        bool operator==(const Move &other) const {
            return row == other.row && col == other.col;
        }
    };

    enum class RowType {
        Row,
        Col,
        Diag,
        OppoDiag,
    };

public:
    Game() {
        for (int col = 0; col < COL; ++col) {
            for (int row = 0; row < ROW; ++row) { board_[col][row] = NON_PLAYER; }
        }
    }

    Board board() const {
        return board_;
    }

    void update(int row, int col, int player) {
        board_[col][row] = player;
    }

    void update(const Board &board) {
        board_ = board;
    }

    int at(int row, int col) const {
        return board_[col][row];
    }

    QList<int> get_row(RowType row_type, Move move) const {
        QList<int> row;
        switch (row_type) {
            case RowType::Row: {
                for (int j = 0; j < COL; j++) { row.append(board_[j][move.row]); }
            } break;
            case RowType::Col: {
                for (int i = 0; i < ROW; i++) { row.append(board_[move.col][i]); }
            } break;
            case RowType::Diag: {
                const int n = std::min(move.row, move.col);
                Move      pos{move.row - n, move.col - n};
                while (pos.row < ROW && pos.col < COL) {
                    row.append(board_[pos.col][pos.row]);
                    ++pos.row;
                    ++pos.col;
                }
            } break;
            case RowType::OppoDiag: {
                const int n = std::min(ROW - 1 - move.row, move.col);
                Move      pos{move.row + n, move.col - n};
                while (pos.row >= 0 && pos.col < COL) {
                    row.append(board_[pos.col][pos.row]);
                    --pos.row;
                    ++pos.col;
                }
            } break;
        }
        return row;
    }

    bool check(const QList<int> &row, int player) const {
        int total_placed = 0;
        for (const int stone : row) {
            if (stone == player) {
                if (++total_placed >= 4) { break; }
            } else {
                total_placed = 0;
            }
        }
        return total_placed >= 4;
    }

    void add_player(int player) {
        const int index = players_.indexOf(player);
        if (index != -1) { players_.removeAt(index); }
        players_.append(player);
    }

    int get_previous_player() const {
        return players_.back();
    }

    int get_current_player() const {
        return players_.front();
    }

    void next_turn() {
        add_player(get_current_player());
    }

    bool is_dead_heat() const {
        for (int col = 0; col < COL; ++col) {
            for (int row = 0; row < ROW; ++row) {
                if (board_[col][row] == NON_PLAYER) { return false; }
            }
        }
        return true;
    }

    EvalResult evolve(Move move, bool no_place = false) {
        const int player = get_current_player();
        if (!no_place) {
            board_[move.col][move.row] = player;
            next_turn();
        }

        bool done   = false;
        int  winner = NON_PLAYER;

        if (check(get_row(RowType::Row, move), player) || check(get_row(RowType::Col, move), player)
            || check(get_row(RowType::Diag, move), player) || check(get_row(RowType::OppoDiag, move), player)) {
            done   = true;
            winner = player;
        } else {
            done = is_dead_heat();
        }

        return {done, winner};
    }

    EvalResult test_evolve(Move move, int player) {
        const int last_stone       = board_[move.col][move.row];
        board_[move.col][move.row] = player;

        bool done   = false;
        int  winner = NON_PLAYER;

        if (check(get_row(RowType::Row, move), player) || check(get_row(RowType::Col, move), player)
            || check(get_row(RowType::Diag, move), player) || check(get_row(RowType::OppoDiag, move), player)) {
            done   = true;
            winner = player;
        } else {
            done = is_dead_heat();
        }

        board_[move.col][move.row] = last_stone;

        return {done, winner};
    }

    QList<Move> get_valid_moves() const {
        QList<Move> moves;
        for (int j = 0; j < COL; j++) {
            for (int i = 0; i < ROW; i++) {
                if (board_[j][i] == NON_PLAYER) {
                    moves.append({i, j});
                    break;
                }
            }
        }
        return moves;
    }

private:
    Board      board_;
    QList<int> players_;
};

class MctsNode : std::enable_shared_from_this<MctsNode> {
public:
    using Move = Game::Move;

    struct Data {
        Move   move;
        double prob;
    };

    struct SelectResult {
        Move                      move;
        std::shared_ptr<MctsNode> node;
    };

public:
    MctsNode(MctsNode *parent, double prior_prob)
        : parent_(parent)
        , prior_prob_(prior_prob)
        , visit_count_(0)
        , value_sum_(0)
        , expanded_(false)
        , c_puct_(5.0) {}

    bool is_leaf() const {
        return succs_.empty();
    }

    bool is_root() const {
        return parent_ == nullptr;
    }

    void expand(const QList<Data> &guide) {
        if (expanded_) { return; }
        for (const auto &[move, prob] : guide) {
            indices_[move] = succs_.size();
            succs_.append(std::make_shared<MctsNode>(this, prob));
        }
        expanded_ = true;
    }

    void backup(double leaf_val) {
        if (!is_root()) { parent_->backup(-leaf_val); }
        ++visit_count_;
        value_sum_ += leaf_val;
    }

    SelectResult select() const {
        Q_ASSERT(!indices_.empty());
        Move best_move = indices_.firstKey();
        for (const auto &[move, index] : indices_.asKeyValueRange()) {
            if (succs_[index]->get_value() > succs_[indices_[best_move]]->get_value()) { best_move = move; }
        }
        return {best_move, succs_[indices_[best_move]]};
    }

    double get_value() const {
        const double value = visit_count_ == 0 ? 0 : value_sum_ / visit_count_;
        const double ucb   = c_puct_ * prior_prob_ * sqrt(parent_->visit_count_) / (1 + visit_count_);
        return value + ucb;
    }

    Move get_move(double temp = 0) const {
        Q_ASSERT(is_root());
        Q_ASSERT(temp >= 0 && temp <= 1);
        if (temp > 1e-6) {
            QList<Move> moves;
            QList<int>  visit_counts;
            for (const auto &[move, index] : indices_.asKeyValueRange()) {
                moves.append(move);
                visit_counts.append(succs_[index]->visit_count_);
            }
            QList<double> probs(visit_counts.begin(), visit_counts.end());
            for (auto &prob : probs) { prob = 1.0 / temp * log(prob + 1e-10); }
            probs = softmax(probs);
            return moves[choice(probs)];
        } else {
            int                 best_visit_count = std::numeric_limits<int>::lowest();
            std::optional<Move> best_move;
            for (const auto &[move, index] : indices_.asKeyValueRange()) {
                if (const auto &succ = succs_[index]; succ->visit_count_ > best_visit_count) {
                    best_visit_count = succ->visit_count_;
                    best_move        = move;
                }
            }
            return best_move.value();
        }
    }

private:
    MctsNode                        *parent_;
    QMap<Move, int>                  indices_;
    QList<std::shared_ptr<MctsNode>> succs_;
    double                           prior_prob_;
    int                              visit_count_;
    double                           value_sum_;
    bool                             expanded_;
    double                           c_puct_;
};

static QList<MctsNode::Data> get_guide_by_random_policy(const QList<MctsNode::Move> &moves) {
    const double          prob = moves.empty() ? 0.0 : 1.0 / moves.size();
    QList<MctsNode::Data> guide;
    for (const auto &move : moves) { guide.append({move, prob}); }
    return guide;
}

static MctsNode::Move sample(const QList<MctsNode::Data> &guide) {
    QList<double> probs;
    for (const auto &[_, prob] : guide) { probs.append(prob); }
    return guide[choice(probs)].move;
}

static double rollout(Game &board) {
    const int player = board.get_current_player();
    while (true) {
        const auto move           = sample(get_guide_by_random_policy(board.get_valid_moves()));
        const auto [done, winner] = board.evolve(move);
        if (!done) { continue; }
        if (winner == Game::NON_PLAYER) { return 0; }
        return winner == player ? 1 : -1;
    }
}

static void run_mcts_once(const Game &board, std::shared_ptr<MctsNode> root) {
    auto cloned_board = board;
    auto node         = root;

    bool done   = false;
    int  winner = Game::NON_PLAYER;

    while (!node->is_leaf()) {
        const auto select_result = node->select();
        node                     = select_result.node;
        const auto evolve_result = cloned_board.evolve(select_result.move);
        done                     = evolve_result.done;
        winner                   = evolve_result.winner;
    }

    double neg_leaf_value = 0;
    if (node->is_root() || !done) {
        const auto valid_moves = cloned_board.get_valid_moves();
        node->expand(get_guide_by_random_policy(valid_moves));
        neg_leaf_value = rollout(cloned_board);
    } else if (winner == Game::NON_PLAYER) {
        neg_leaf_value = 0;
    } else {
        neg_leaf_value = winner == cloned_board.get_current_player() ? 1 : -1;
    }

    node->backup(-neg_leaf_value);
}

QDebug operator<<(QDebug dbg, const Game &game) {
    QDebugStateSaver saver(dbg);
    for (int i = game.ROW - 1; i >= 0; --i) {
        for (int j = 0; j < game.COL; ++j) { dbg.space() << game.at(i, j); }
        if (i != 0) { dbg.nospace() << "\n"; }
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, const Game::Move &move) {
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Move(" << move.row << ", " << move.col << ")";
    return dbg;
}

static cv::Mat crop_image(const cv::Mat &src, const MaaRect &rect) {
    return src.rowRange(rect.y, rect.y + rect.height).colRange(rect.x, rect.x + rect.width);
}

bool SolveFourInRow::parse_params(SolveFourInRowParam &param_out, MaaStringView raw_param) {
    using Mode = SolveFourInRowParam::Mode;

    auto opt_param = json::parse(raw_param);
    if (!opt_param.has_value()) { return false; }

    param_out.mcts_iters = opt_param->get("iter", 10000);
    param_out.retry      = opt_param->get("retry", true);

    const auto mode = opt_param->get("mode", "black");
    if (false) {
    } else if (mode == "black") {
        param_out.mode = Mode::Black;
    } else if (mode == "white") {
        param_out.mode = Mode::White;
    } else if (mode == "random") {
        param_out.mode = Mode::Random;
    } else {
        param_out.mode = Mode::Black;
    }

    return true;
}

coro::Promise<bool> SolveFourInRow::solve_four_in_row(
    std::shared_ptr<SyncContext> context,
    MaaStringView                task_name,
    MaaStringView                param,
    const MaaRect               &cur_box,
    MaaStringView                cur_rec_detail) {
    SolveFourInRowParam opt;
    if (!parse_params(opt, param)) {
        qCritical().noquote().nospace() << task_name << ": invalid arguments";
        co_return false;
    }
    if (opt.mode == SolveFourInRowParam::Mode::Random) {
        qCritical().nospace().noquote() << task_name << ": random mode is not supported yet";
        co_return true;
    }

    const MaaRect roi{515, 145, 505, 430};

    const int black_store_rgb[3]{9, 16, 16};
    const int white_store_rgb[3]{236, 242, 242};
    const int cell_width  = roi.width / Game::COL;
    const int cell_height = roi.height / Game::ROW;

    //! TODO: process param 'mode'
    const int black_stone = 1;
    const int white_stone = 2;
    const int my_stone    = opt.mode == SolveFourInRowParam::Mode::Black ? black_stone : white_stone;
    const int ai_stone    = opt.mode == SolveFourInRowParam::Mode::Black ? white_stone : black_stone;

    auto get_board_state = [=](std::shared_ptr<details::Image> screen) {
        context->screencap(screen).sync_wait();
        const auto im = crop_image(cv::Mat(screen->height(), screen->width(), screen->type(), screen->raw_data()), roi);

        Game::Board board;
        for (int row = 0; row < Game::ROW; ++row) {
            for (int col = 0; col < Game::COL; ++col) {
                const auto pixel = im.at<cv::Vec3b>(row * cell_height + cell_height / 2, col * cell_width + cell_width / 2);
                const int  rgb[3]{pixel[0], pixel[1], pixel[2]};
                if (const double distance = eval_color_distance(black_store_rgb, rgb); distance < 64) {
                    board[col][Game::ROW - 1 - row] = black_stone;
                } else if (const double distance = eval_color_distance(white_store_rgb, rgb); distance < 64) {
                    board[col][Game::ROW - 1 - row] = white_stone;
                } else {
                    board[col][Game::ROW - 1 - row] = Game::NON_PLAYER;
                }
            }
        }

        return board;
    };

    auto wait = [] {
        QElapsedTimer tm;
        tm.start();
        while (tm.elapsed() < 2000) { std::this_thread::yield(); }
    };

    bool reenter = false;
    auto screen  = details::Image::make();

    while (true) {
        bool done       = false;
        int  winner     = Game::NON_PLAYER;
        bool terminated = false;

        if (reenter) {
            qInfo() << "restart to solve four-in-a-row";
        } else {
            qInfo() << "start to solve four-in-a-row";
        }

        std::optional<Game::Board> opt_last_board;
        while (!done) {
            if (!opt_last_board.has_value() && opt.mode == SolveFourInRowParam::Mode::White) { wait(); }

            const auto board = get_board_state(screen);

            Game game;
            game.update(board);
            game.add_player(my_stone);
            game.add_player(ai_stone);

            if (opt_last_board.has_value()) {
                const auto &last_board = opt_last_board.value();
                bool        has_diff   = false;
                for (int col = 0; col < Game::COL && !done; ++col) {
                    for (int row = 0; row < Game::ROW && !done; ++row) {
                        const int stone      = board[col][row];
                        const int last_stone = last_board[col][row];
                        if (stone == Game::NON_PLAYER) { break; }
                        if (stone != last_stone) {
                            if (const int prev_player = game.get_previous_player(); stone == prev_player) {
                                const Game::Move prev_move{row, col};
                                qInfo() << "[detect] player" << prev_player << "drop" << prev_move;
                                const auto evolve_result = game.evolve(prev_move, true);
                                done                     = evolve_result.done;
                                winner                   = evolve_result.winner;
                            }
                            has_diff = true;
                        }
                    }
                }
                if (!has_diff) {
                    terminated = true;
                    break;
                }
            } else if (game.is_dead_heat()) {
                done = true;
            }

            if (done) { break; }

            qInfo().nospace() << "current board state\n" << game;

            auto root = std::make_shared<MctsNode>(nullptr, 1.0);
            for (int i = 0; i < opt.mcts_iters; ++i) { run_mcts_once(game, root); }

            const auto mcts_best_move = root->get_move();

            auto valid_moves_before_drop = game.get_valid_moves();
            valid_moves_before_drop.removeOne(mcts_best_move);

            auto move = mcts_best_move;
            for (const auto &oppo_valid_move : valid_moves_before_drop) {
                const auto test_evolve_result = game.test_evolve(oppo_valid_move, ai_stone);
                if (test_evolve_result.done && test_evolve_result.winner == ai_stone) {
                    move = oppo_valid_move;
                    break;
                }
            }

            Game::EvalResult evolve_result;
            if (move != mcts_best_move) {
                qInfo() << "cancel mcts best drop" << mcts_best_move << "to avoid being beaten";
                evolve_result = game.evolve(move);
            } else {
                const auto saved_board = game.board();
                evolve_result          = game.evolve(mcts_best_move);
                if (!evolve_result.done && mcts_best_move.row + 1 < Game::ROW && !valid_moves_before_drop.empty()) {
                    //! NOTE: test ai drop with greedy policy and decide whether to drop or not
                    const Game::Move prob_oppo_move{mcts_best_move.row + 1, mcts_best_move.col};
                    const auto       test_evolve_result = game.test_evolve(prob_oppo_move, ai_stone);
                    if (test_evolve_result.done && test_evolve_result.winner == ai_stone) {
                        qInfo() << "predict ai drop with greedy policy, cancel mcts best drop";
                        game.update(saved_board);
                        game.add_player(ai_stone);
                        move          = valid_moves_before_drop[choice(0, valid_moves_before_drop.size() - 1)];
                        evolve_result = game.evolve(move);
                    }
                }
            }

            done   = evolve_result.done;
            winner = evolve_result.winner;

            opt_last_board = game.board();

            qInfo() << "player" << game.get_current_player() << "drop" << move;

            const int click_pos_x = roi.x + move.col * cell_width + cell_width / 2;
            const int click_pos_y = roi.y + (Game::ROW - 1 - move.row) * cell_height + cell_height / 2;
            co_await context->click(click_pos_x, click_pos_y);

            wait();
        }

        //! FIXME: in terminated state, sometimes we need quit the finished stage, but sometimes we don't

        if (terminated) {
            //! NOTE: generally it indicates the failure of the game
            qWarning() << "game is terminated unexpectedly";
        } else if (winner == Game::NON_PLAYER) {
            qInfo() << "game ends in a tie";
        } else {
            qInfo() << "player" << winner << "wins";
        }

        //! NOTE: maybe fail when terminated, but it doesn't matter
        //! FIXME: possible to fall through before the specified task finished in the emulator device
        co_await context->run_task("Company.QuitFourInRowFinishedStage");

        if ((terminated || winner != my_stone) && opt.retry) {
            qInfo() << "retry after termination";
            reenter = true;
            //! NOTE: let it fallthrough if not at the start page
            co_await context->run_task("Company.StartFourInRow");
            continue;
        }

        break;
    }

    co_return true;
}

} // namespace Action
