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

#pragma once

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QUrl>
#include <gsl/gsl>

namespace Experimental {

struct PackageInfo {
    QString     name;
    QString     author;
    QString     version;
    QString     description;
    QStringList resolutions;
    QString     icon_url;

    QString generic_version() const {
        return version.startsWith('v') ? version : 'v' + version;
    }

    bool supports_resolution(int width, int height) const {
        return supports_resolution(QString("%1x%2").arg(width, height));
    };

    bool supports_resolution(const QString &resolution) const {
        return resolutions.contains(resolution);
    };

    QStringList authors() const {
        QStringList author_list;
        for (const auto &item : author.split(";")) { author_list.append(item.trimmed()); }
        return author_list;
    }
};

class Package {
public:
    static inline const char *INFO_FILE   = "package.json";
    static inline const char *DIGEST_FILE = "package.sha";

public:
    static std::shared_ptr<Package> create(const QString &package_dir);

    void    set_location(const QString &package_dir);
    QString location() const;
    bool    valid() const;
    auto    info() const -> std::reference_wrapper<const PackageInfo>;
    bool    digest_available() const;
    QString digest();
    void    ensure_sha_generated();
    bool    load_info();

protected:
    Package() = default;

private:
    QString                    package_dir_;
    std::optional<PackageInfo> opt_info_;
    std::optional<QString>     opt_digest_;
};

} // namespace Experimental
