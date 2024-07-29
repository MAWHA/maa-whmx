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

#include "Settings.h"
#include "../Consts.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>

namespace UI {

Settings::Settings(QWidget *parent)
    : QWidget(parent) {
    setup();
}

Settings::~Settings() {}

void Settings::setup() {
    auto version    = new QLabel;
    auto note       = new QLabel;
    auto down_arrow = new QLabel;
    auto href       = new QLabel;
    auto up_arrow   = new QLabel;
    auto hint       = new QLabel;

    version->setText(QString("当前版本：%1").arg(QApplication::applicationVersion()));
    {
        auto font = version->font();
        font.setPointSize(16);
        version->setFont(font);
    }
    note->setText("『　不用找了，没做！——正式版本再加设置项！　』");
    down_arrow->setText("↓");
    {
        const auto url       = QString::fromUtf8(Consts::HOMEPAGE_URL);
        const auto text      = "~灌注五花米线谢谢喵~";
        const auto html_text = QString(R"(<a style="text-decoration: none;" href="%1">%2</a>)").arg(url).arg(text);
        href->setText(html_text);
    }
    href->setOpenExternalLinks(true);
    up_arrow->setText("↑↑↑");
    hint->setText("点我点我～(∠・ω< )⌒☆");

    version->setAlignment(Qt::AlignCenter);
    note->setAlignment(Qt::AlignCenter);
    down_arrow->setAlignment(Qt::AlignCenter);
    href->setAlignment(Qt::AlignCenter);
    up_arrow->setAlignment(Qt::AlignCenter);
    hint->setAlignment(Qt::AlignCenter);

    auto layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(version);
    layout->addSpacerItem(new QSpacerItem(0, 16));
    layout->addWidget(note);
    layout->addWidget(down_arrow);
    layout->addWidget(href);
    layout->addWidget(up_arrow);
    layout->addWidget(hint);
    layout->addStretch();
}

} // namespace UI
