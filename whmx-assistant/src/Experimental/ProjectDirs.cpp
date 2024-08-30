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

#include "ProjectDirs.h"

#include <QtWidgets/QApplication>
#include <QtCore/QStandardPaths>
#include <gsl/gsl>

namespace Experimental {

void copy_folder(const QString &src, const QString &dst) {
    QDir dir(src);
    if (!dir.exists()) { return; }

    for (const auto &d : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dst_path = dst + QDir::separator() + d;
        dir.mkpath(dst_path);
        copy_folder(src + QDir::separator() + d, dst_path);
    }

    for (const auto &f : dir.entryList(QDir::Files)) { QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f); }
}

QDir ProjectDirs::home() {
    return QApplication::applicationDirPath();
}

QDir ProjectDirs::create_or_get(const QString &dir) {
    auto d = home();
    if (!d.exists(dir)) {
        const bool ok = d.mkdir(dir);
        Ensures(ok);
    }
    const bool ok = d.cd(dir);
    Ensures(ok);
    return d;
}

QDir ProjectDirs::app_data() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

} // namespace Experimental
