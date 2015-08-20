/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <QtTest/QtTest>

class TEST_NAME : public QObject
{
    Q_OBJECT

public:
    TEST_NAME();
private slots:
    void runAllTests();
};

TEST_NAME::TEST_NAME()
{
}

void TEST_NAME::runAllTests()
{
    int retVal = RUN_ALL_TESTS();
    QCOMPARE(retVal, 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    TEST_NAME gTest;
    return QTest::qExec(&gTest, argc, argv);
}

#include "testmain.moc"
