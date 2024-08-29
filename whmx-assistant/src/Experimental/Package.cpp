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

#include "Package.h"

#include <QtCore/QDir>
#include <QtCore/QCryptographicHash>
#include <QtCore/QRegularExpression>
#include <meojson/json.hpp>

namespace Experimental {

std::shared_ptr<Package> Package::create(const QString &package_dir) {
    Expects(!package_dir.isEmpty());
    auto package = std::shared_ptr<Package>(new Package);
    package->set_location(package_dir);
    return package;
}

void Package::set_location(const QString &package_dir) {
    Expects(!package_dir.isEmpty());
    if (package_dir_ == package_dir) { return; }
    opt_info_.reset();
    opt_digest_.reset();
    package_dir_ = package_dir;
}

QString Package::location() const {
    Expects(!package_dir_.isEmpty());
    return package_dir_;
}

bool Package::valid() const {
    return opt_info_.has_value();
}

std::reference_wrapper<const PackageInfo> Package::info() const {
    Ensures(valid());
    return opt_info_.value();
}

bool Package::digest_available() const {
    if (!valid()) { return false; }
    if (opt_digest_.has_value()) { return true; }

    const QDir      package_dir(package_dir_);
    const QFileInfo file_info(package_dir.absoluteFilePath(DIGEST_FILE));
    return file_info.exists() && file_info.isFile() && file_info.isReadable();
}

QString Package::digest() {
    Expects(digest_available());
    if (!opt_digest_.has_value()) {
        const QDir package_dir(package_dir_);
        QFile      file(package_dir.absoluteFilePath(DIGEST_FILE));
        Ensures(file.open(QIODevice::ReadOnly));
        opt_digest_ = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256).toHex();
    }
    Ensures(opt_digest_.has_value());
    return opt_digest_.value();
}

void Package::ensure_sha_generated() {
    Expects(valid());

    const QDir package_dir(package_dir_);

    if (!package_dir.exists(DIGEST_FILE)) {
        QList<QPair<QString, QString>> sha_list;
        QList<QDir>                    stack;
        stack.append(package_dir);
        while (!stack.empty()) {
            const auto dir = stack.takeLast();
            for (const auto &entry : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                stack.append(QDir(dir.absoluteFilePath(entry)));
            }
            for (const auto &entry : dir.entryList(QDir::Files)) {
                const auto file_path = dir.absoluteFilePath(entry);
                const auto item      = package_dir.relativeFilePath(file_path);
                if (item == INFO_FILE) { continue; }
                QFile file(file_path);
                Expects(file.open(QIODevice::ReadOnly));
                const auto sha = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256).toHex();
                sha_list.append({item, sha});
            }
        }
        std::sort(sha_list.begin(), sha_list.end());
        QFile sha_file(package_dir.absoluteFilePath(DIGEST_FILE));
        Expects(sha_file.open(QIODevice::WriteOnly | QIODevice::Text));
        QStringList entries;
        for (const auto &[item, sha] : sha_list) { entries.append(sha + ' ' + item); }
        sha_file.write(entries.join("\n").toUtf8());
    }

    const QFileInfo file_info(package_dir.absoluteFilePath(DIGEST_FILE));
    Ensures(file_info.exists() && file_info.isFile() && file_info.isReadable());
}

bool Package::load_info() {
    const QDir dir(package_dir_);
    if (!dir.exists(INFO_FILE)) { return false; }

    QFile file(dir.absoluteFilePath(INFO_FILE));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { return false; }

    const auto opt_json = json::parse(file.readAll());
    if (!opt_json.has_value()) { return false; }

    PackageInfo info;

    const auto &data = opt_json.value();
    if (!data.is_object()) { return false; }

    if (data.contains("name") && data.at("name").is_string()) {
        info.name = QString::fromUtf8(data.at("name").as_string()).trimmed();
    } else {
        return false;
    }

    if (data.contains("version") && data.at("version").is_string()) {
        info.version = QString::fromUtf8(data.at("version").as_string());
    } else {
        return false;
    }
    if (const QRegularExpression re(R"(^v?\d+(\.\d+){1,3}([-_\.](beta|alpha|rc)(\.\d+)?)?$)");
        !re.match(info.version).hasMatch()) {
        return false;
    }

    if (data.contains("description")) {
        if (data.at("description").is_string()) {
            info.description = QString::fromUtf8(data.at("description").as_string()).trimmed();
        } else {
            return false;
        }
    }

    if (data.contains("author")) {
        if (data.at("author").is_string()) {
            info.author = QString::fromUtf8(data.at("author").as_string());
        } else if (data.at("author").is_array()) {
            if (!data.at("author").all<std::string>()) { return false; }
            QStringList authors;
            for (const auto &item : data.at("author").as_array()) {
                const auto author = QString::fromUtf8(item.as_string()).trimmed();
                if (author.isEmpty()) { continue; }
                if (author.contains(";")) { return false; }
                authors.append(author);
            }
            info.author = authors.join(";");
        } else {
            return false;
        }
    }

    if (data.contains("resolutions")) {
        if (!data.at("resolutions").all<std::string>()) { return false; }
        const QRegularExpression re(R"(^[1-9]\d*[x:][1-9]\d*$)");
        for (const auto &item : data.at("resolutions").as_array()) {
            const auto resolution = QString::fromUtf8(item.as_string());
            if (!re.match(resolution).hasMatch()) { return false; }
            info.resolutions.append(resolution);
        }
    }

    if (data.contains("icon")) {
        if (!data.at("icon").is_string()) { return false; }
        const auto icon_url = QString::fromUtf8(data.at("icon").as_string());
        if (dir.exists(icon_url)) {
            info.icon_url = dir.absoluteFilePath(icon_url);
        } else {
            info.icon_url = icon_url;
        }
    }

    opt_info_ = info;

    return true;
}

} // namespace Experimental
