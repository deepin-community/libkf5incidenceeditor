/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "conflictresolvertest.h"
#include "conflictresolver.h"

#include <KCalendarCore/Duration>
#include <KCalendarCore/Event>
#include <KCalendarCore/Period>

#include <QTest>
#include <QWidget>

using namespace IncidenceEditorNG;

void ConflictResolverTest::insertAttendees()
{
    for (const CalendarSupport::FreeBusyItem::Ptr &item : std::as_const(attendees)) {
        resolver->insertAttendee(item);
    }
}

void ConflictResolverTest::addAttendee(const QString &email, const KCalendarCore::FreeBusy::Ptr &fb, KCalendarCore::Attendee::Role role)
{
    QString name = QStringLiteral("attendee %1").arg(attendees.count());
    CalendarSupport::FreeBusyItem::Ptr item(
        new CalendarSupport::FreeBusyItem(KCalendarCore::Attendee(name, email, false, KCalendarCore::Attendee::Accepted, role), nullptr));
    item->setFreeBusy(KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(*fb.data())));
    attendees << item;
}

void ConflictResolverTest::initTestCase()
{
    parent = new QWidget;
    init();
}

void ConflictResolverTest::init()
{
    base = QDateTime::currentDateTime().addDays(1);
    end = base.addSecs(10 * 60 * 60);
    resolver = new ConflictResolver(parent, parent);
}

void ConflictResolverTest::cleanup()
{
    delete resolver;
    resolver = nullptr;
    attendees.clear();
}

void ConflictResolverTest::simpleTest()
{
    KCalendarCore::Period meeting(end.addSecs(-3 * 60 * 60), KCalendarCore::Duration(2 * 60 * 60));
    addAttendee(QStringLiteral("albert@einstein.net"), KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << meeting)));

    insertAttendees();

    static const int resolution = 15 * 60;
    resolver->setResolution(resolution);
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    resolver->findAllFreeSlots();

    QVERIFY(resolver->availableSlots().size() == 2);

    KCalendarCore::Period first = resolver->availableSlots().at(0);
    QCOMPARE(first.start(), base);
    QCOMPARE(first.end(), meeting.start());

    KCalendarCore::Period second = resolver->availableSlots().at(1);
    QEXPECT_FAIL("", "Got broken in revision f17b9a8c975588ad7cf4ce8b94ab8e32ac193ed8", Continue);
    QCOMPARE(second.start(), meeting.end().addSecs(resolution)); // add 15 minutes because the
    // free block doesn't start until
    // the next timeslot
    QCOMPARE(second.end(), end);
}

void ConflictResolverTest::stillPrettySimpleTest()
{
    KCalendarCore::Period meeting1(base, KCalendarCore::Duration(2 * 60 * 60));
    KCalendarCore::Period meeting2(base.addSecs(60 * 60), KCalendarCore::Duration(2 * 60 * 60));
    KCalendarCore::Period meeting3(end.addSecs(-3 * 60 * 60), KCalendarCore::Duration(2 * 60 * 60));
    addAttendee(QStringLiteral("john.f@kennedy.com"),
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << meeting1 << meeting3)));
    addAttendee(QStringLiteral("elvis@rock.com"),
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << meeting2 << meeting3)));
    addAttendee(QStringLiteral("albert@einstein.net"), KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << meeting3)));

    insertAttendees();

    static const int resolution = 15 * 60;
    resolver->setResolution(resolution);
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    resolver->findAllFreeSlots();

    QVERIFY(resolver->availableSlots().size() == 2);

    KCalendarCore::Period first = resolver->availableSlots().at(0);
    QEXPECT_FAIL("", "Got broken in revision f17b9a8c975588ad7cf4ce8b94ab8e32ac193ed8", Continue);
    QCOMPARE(first.start(), meeting2.end().addSecs(resolution));
    QCOMPARE(first.end(), meeting3.start());

    KCalendarCore::Period second = resolver->availableSlots().at(1);
    QEXPECT_FAIL("", "Got broken in revision f17b9a8c975588ad7cf4ce8b94ab8e32ac193ed8", Continue);
    QCOMPARE(second.start(), meeting3.end().addSecs(resolution)); // add 15 minutes because the
    // free block doesn't start until
    // the next timeslot
    QCOMPARE(second.end(), end);
}

#define _time(h, m) QDateTime(base.date(), QTime(h, m))

void ConflictResolverTest::akademy2010()
{
    // based off akademy 2010 schedule

    // first event was at 9:30, so lets align our start time there
    base.setTime(QTime(9, 30));
    end = base.addSecs(8 * 60 * 60);
    KCalendarCore::Period opening(_time(9, 30), _time(9, 45));
    KCalendarCore::Period keynote(_time(9, 45), _time(10, 30));

    KCalendarCore::Period sevenPrinciples(_time(10, 30), _time(11, 15));
    KCalendarCore::Period commAsService(_time(10, 30), _time(11, 15));

    KCalendarCore::Period kdeForums(_time(11, 15), _time(11, 45));
    KCalendarCore::Period oviStore(_time(11, 15), _time(11, 45));

    // 10 min break

    KCalendarCore::Period highlights(_time(12, 0), _time(12, 45));
    KCalendarCore::Period styles(_time(12, 0), _time(12, 45));

    KCalendarCore::Period wikimedia(_time(12, 45), _time(13, 15));
    KCalendarCore::Period avalanche(_time(12, 45), _time(13, 15));

    KCalendarCore::Period pimp(_time(13, 15), _time(13, 45));
    KCalendarCore::Period direction(_time(13, 15), _time(13, 45));

    // lunch 1 hr 25 min lunch

    KCalendarCore::Period blurr(_time(15, 15), _time(16, 00));
    KCalendarCore::Period plasma(_time(15, 15), _time(16, 00));

    //  for ( int i = 1; i < 80; ++i ) {
    // adds 80 people (adds the same 8 people 10 times)
    addAttendee(
        QStringLiteral("akademyattendee1@email.com"),
        KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << keynote << oviStore << wikimedia << direction)));
    addAttendee(
        QStringLiteral("akademyattendee2@email.com"),
        KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << keynote << commAsService << highlights << pimp)));
    addAttendee(QStringLiteral("akademyattendee3@email.com"),
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << kdeForums << styles << pimp << plasma)));
    addAttendee(QStringLiteral("akademyattendee4@email.com"),
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << keynote << oviStore << pimp << blurr)));
    addAttendee(QStringLiteral("akademyattendee5@email.com"),
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << keynote << oviStore << highlights << avalanche)));
    addAttendee(QStringLiteral("akademyattendee6@email.com"),
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << keynote << commAsService << highlights)));
    addAttendee(QStringLiteral("akademyattendee7@email.com"),
                KCalendarCore::FreeBusy::Ptr(
                    new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << kdeForums << styles << avalanche << pimp << plasma)));
    addAttendee(
        QStringLiteral("akademyattendee8@email.com"),
        KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << keynote << oviStore << wikimedia << blurr)));
    //  }

    insertAttendees();

    const int resolution = 5 * 60;
    resolver->setResolution(resolution);
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    // QBENCHMARK {
    resolver->findAllFreeSlots();
    // }

    QVERIFY(resolver->availableSlots().size() == 3);
    QEXPECT_FAIL("", "Got broken in revision f17b9a8c975588ad7cf4ce8b94ab8e32ac193ed8", Abort);
    QCOMPARE(resolver->availableSlots().at(0).duration(), KCalendarCore::Duration(10 * 60));
    QCOMPARE(resolver->availableSlots().at(1).duration(), KCalendarCore::Duration(1 * 60 * 60 + 25 * 60));
    QVERIFY(resolver->availableSlots().at(2).start() > plasma.end());
}

void ConflictResolverTest::testPeriodIsLargerThenTimeframe()
{
    base.setDate(QDate(2010, 7, 29));
    base.setTime(QTime(7, 30));

    end.setDate(QDate(2010, 7, 29));
    end.setTime(QTime(8, 30));

    KCalendarCore::Period testEvent(_time(5, 45), _time(8, 45));

    addAttendee(QStringLiteral("kdabtest1@demo.kolab.org"),
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << testEvent)));
    addAttendee(QStringLiteral("kdabtest2@demo.kolab.org"), KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List())));

    insertAttendees();
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    resolver->findAllFreeSlots();

    QCOMPARE(resolver->availableSlots().size(), 0);
}

void ConflictResolverTest::testPeriodBeginsBeforeTimeframeBegins()
{
    base.setDate(QDate(2010, 7, 29));
    base.setTime(QTime(7, 30));

    end.setDate(QDate(2010, 7, 29));
    end.setTime(QTime(9, 30));

    KCalendarCore::Period testEvent(_time(5, 45), _time(8, 45));

    addAttendee(QStringLiteral("kdabtest1@demo.kolab.org"),
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << testEvent)));
    addAttendee(QStringLiteral("kdabtest2@demo.kolab.org"), KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List())));

    insertAttendees();
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    resolver->findAllFreeSlots();

    QCOMPARE(resolver->availableSlots().size(), 1);
    KCalendarCore::Period freeslot = resolver->availableSlots().at(0);
    QCOMPARE(freeslot.start(), _time(8, 45));
    QCOMPARE(freeslot.end(), end);
}

void ConflictResolverTest::testPeriodEndsAfterTimeframeEnds()
{
    base.setDate(QDate(2010, 7, 29));
    base.setTime(QTime(7, 30));

    end.setDate(QDate(2010, 7, 29));
    end.setTime(QTime(9, 30));

    KCalendarCore::Period testEvent(_time(8, 00), _time(9, 45));

    addAttendee(QStringLiteral("kdabtest1@demo.kolab.org"),
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << testEvent)));
    addAttendee(QStringLiteral("kdabtest2@demo.kolab.org"), KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List())));

    insertAttendees();
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    resolver->findAllFreeSlots();

    QCOMPARE(resolver->availableSlots().size(), 1);
    KCalendarCore::Period freeslot = resolver->availableSlots().at(0);
    QCOMPARE(freeslot.duration(), KCalendarCore::Duration(30 * 60));
    QCOMPARE(freeslot.start(), base);
    QCOMPARE(freeslot.end(), _time(8, 00));
}

void ConflictResolverTest::testPeriodEndsAtSametimeAsTimeframe()
{
    base.setDate(QDate(2010, 7, 29));
    base.setTime(QTime(7, 45));

    end.setDate(QDate(2010, 7, 29));
    end.setTime(QTime(8, 45));

    KCalendarCore::Period testEvent(_time(5, 45), _time(8, 45));

    addAttendee(QStringLiteral("kdabtest1@demo.kolab.org"),
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << testEvent)));
    addAttendee(QStringLiteral("kdabtest2@demo.kolab.org"), KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List())));

    insertAttendees();
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    resolver->findAllFreeSlots();

    QCOMPARE(resolver->availableSlots().size(), 0);
}

QTEST_MAIN(ConflictResolverTest)
