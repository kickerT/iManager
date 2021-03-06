
/****************************************************************************
*  Copyright (C) 2011 Rapha�l MARQUES <work.rmarques@gmail.com>
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

/** --------------------------------------------------------------------------------------------------------------- **/

#ifndef CONTACTS_HPP
#define CONTACTS_HPP

/** --------------------------------------------------------------------------------------------------------------- **/

#include <qdebug.h>

#include <qobject.h>
#include <qwidget.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qtreewidget.h>

#include <qstatemachine.h>
#include <qgraphicsview.h>
#include <qgraphicsscene.h>
#include <qgraphicswidget.h>
#include <qpropertyanimation.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicslinearlayout.h>
#include <qparallelanimationgroup.h>
#include <qsequentialanimationgroup.h>

/** --------------------------------------------------------------------------------------------------------------- **/

#include "options.hpp"
#include "webview.hpp"
#include "database.hpp"
#include "searchbar.hpp"

/** --------------------------------------------------------------------------------------------------------------- **/

class CONTACTS : public QWidget {

    Q_OBJECT

public:

    CONTACTS(QWidget * parent = 0, database * db = 0, const QString & path = "", Options * opt = 0) : QWidget(parent) {
        this -> initAttributes(db, path, opt);
        QHBoxLayout * ContactsLayout = new QHBoxLayout;
        ContactsLayout -> setContentsMargins(0, 0, 0, 0);
        ContactsLayout -> setSpacing(0);

        QVBoxLayout * displayLayout = new QVBoxLayout;
        displayLayout -> setContentsMargins(0, 0, 0, 0);
        displayLayout -> setSpacing(0);

        this -> ContactsTreeList = new QTreeWidget(this);
        this -> ContactsTreeList -> setAlternatingRowColors(true);
        this -> ContactsTreeList -> setSortingEnabled(true);
        this -> ContactsTreeList -> setIconSize(QSize(50, 50));
        this -> ContactsTreeList -> setMinimumWidth(300);
        this -> connect(this -> ContactsTreeList, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(showContact(QTreeWidgetItem *, int)));

        this -> searchContact = new SearchBar(tr("Search ..."), this -> appPath + "/utils/icons", this);
        this -> searchContact -> setEnabled(false);
        this -> connect(this -> searchContact, SIGNAL(hasFocus()), this -> searchContact, SLOT(clear()));
        this -> connect(this -> searchContact, SIGNAL(searchText(const QString &)), this, SLOT(findContacts(const QString &)));
        this -> connect(this -> searchContact, SIGNAL(cleaned()), this, SLOT(loadContactsData()));

        this -> sortContact = new QComboBox(this);
        this -> sortContact -> addItems(QStringList() << tr("Firstname Lastname") << tr("Lastname Firstname"));
        this -> connect(this -> sortContact, SIGNAL(currentIndexChanged(int)), this, SLOT(loadContactsData(int)));

        this -> ContactsWebView = new WebView();
        this -> ContactsWebView -> setFixedSize(364, 483);
        this -> connect(this -> ContactsWebView, SIGNAL(linkClicked(const QUrl &)), this, SLOT(emitSignal(const QUrl &)));

        /////////// ANIMATION ///////////

        QGraphicsProxyWidget * contactProxy = new QGraphicsProxyWidget;
        contactProxy -> setWidget(this -> ContactsWebView);

        QGraphicsWidget * widget = new QGraphicsWidget;
        QGraphicsLinearLayout * layout = new QGraphicsLinearLayout(Qt::Vertical, widget);
        layout -> addItem(contactProxy);
        widget -> setLayout(layout);

        QGraphicsScene * scene = new QGraphicsScene(0, 0, 364, 483, this);
        scene -> setBackgroundBrush(this -> palette().window());
        scene -> addItem(widget);

        QStateMachine * machine = new QStateMachine;
        QState * state1 = new QState(machine);
        QState * state2 = new QState(machine);
        machine -> setInitialState(state1);

        int time(350);
        state1 -> assignProperty(this -> ContactsWebView, "geometry", QRect(0, 0, this -> ContactsWebView -> width(), this -> ContactsWebView -> height()));
        state2 -> assignProperty(this -> ContactsWebView, "geometry", QRect(0, 0, this -> ContactsWebView -> width(), this -> ContactsWebView -> height()));

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        QPropertyAnimation * animation = new QPropertyAnimation(contactProxy, "opacity");
        animation -> setDuration(time);
        animation -> setKeyValueAt(  0, qreal(1));
        animation -> setKeyValueAt(0.5, qreal(0.5));
        animation -> setKeyValueAt(  1, qreal(0));

        QPropertyAnimation * move = new QPropertyAnimation(contactProxy, "pos");
        move -> setDuration(2 * time);
        move -> setKeyValueAt(  0, QPointF(0,         0));
        move -> setKeyValueAt(0.5, QPointF(50,        0));
        move -> setKeyValueAt(1.0, QPointF(120,       0));
        move -> setEasingCurve(QEasingCurve::OutExpo);

        QParallelAnimationGroup * para = new QParallelAnimationGroup;
        para -> addAnimation(animation);
        para -> addAnimation(move);

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        QPropertyAnimation * animation2 = new QPropertyAnimation(contactProxy, "opacity");
        animation2 -> setDuration(time);
        animation2 -> setKeyValueAt(  0, qreal(0));
        animation2 -> setKeyValueAt(0.5, qreal(0.5));
        animation2 -> setKeyValueAt(  1, qreal(1));

        QPropertyAnimation * move2 = new QPropertyAnimation(contactProxy, "pos");
        move2 -> setDuration(2 * time);
        move2 -> setKeyValueAt(  0, QPointF(120,         0));
        move2 -> setKeyValueAt(0.5, QPointF(-70,         0));
        move2 -> setKeyValueAt(1.0, QPointF(0,           0));
        move2 -> setEasingCurve(QEasingCurve::OutExpo);

        QParallelAnimationGroup * para2 = new QParallelAnimationGroup;
        para2 -> addAnimation(animation2);
        para2 -> addAnimation(move2);

        this -> group = new QSequentialAnimationGroup;
        this -> group -> addAnimation(para);
        this -> group -> addAnimation(para2);

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        this -> connect(para, SIGNAL(finished()), this, SLOT(change()));

        QAbstractTransition * t1 = state1 -> addTransition(this -> ContactsTreeList, SIGNAL(itemClicked(QTreeWidgetItem *, int)), state2);
        t1 -> addAnimation(this -> group);

        QAbstractTransition * t2 = state2 -> addTransition(this -> ContactsTreeList, SIGNAL(itemClicked(QTreeWidgetItem *, int)), state1);
        t2 -> addAnimation(this -> group);

        machine -> start();

        this -> view = new QGraphicsView(scene);
        this -> view -> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this -> view -> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this -> view -> setFocusPolicy(Qt::NoFocus);

        ////////////// FIN //////////////

        displayLayout -> addWidget(this -> searchContact);
        displayLayout -> addWidget(this -> ContactsTreeList);
        displayLayout -> addWidget(this -> sortContact);

        ContactsLayout -> addLayout(displayLayout);
        ContactsLayout -> addWidget(this -> view);

        this -> displayDefaultView();
        this -> setLayout(ContactsLayout);
    }

    ~CONTACTS() {}

    /** --------------------------------------------------------------------------------------------------------------- **/

    void displayDefaultView() {
        this -> ContactsTreeList -> setHeaderLabel(tr("There is no contact in the database"));
        this -> contactPicture.load(this -> appPath + "/utils/icons/nobody.png");
        this -> makeDefaultWebPage();
        this -> ContactsWebView -> load(QUrl(this -> tmpDirectory + "/contacts.html"));
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    void findContact(const QString & ID) {
        this -> loadContactsData();
        for(int i(0); i < this -> ContactsTreeList -> topLevelItemCount(); ++i)
            if(this -> ContactsTreeList -> topLevelItem(i) -> text(2) == ID) {
                this -> showContact(this -> ContactsTreeList -> topLevelItem(i));
                this -> searchContact -> setText(this -> ContactsTreeList -> topLevelItem(i) -> text(0));
                break;
            }
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    void displayIcon(bool b = false) {
        this -> display = b;
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    void whoIs(const QString & ID) {
        for(int i(0); i < this -> ContactsTreeList -> topLevelItemCount(); ++i)
            if(this -> ContactsTreeList -> topLevelItem(i) -> text(2) == ID) {
                this -> showContact(this -> ContactsTreeList -> topLevelItem(i));
                this -> group -> start();
            }
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

public slots:

    void loadContactsData(const int & = 0) {
        this -> ContactsTreeList -> clear();
        int cpt(0);

        for(std::vector<people *>::iterator it(this -> DBManager -> book.begin()); it != this -> DBManager -> book.end(); ++it) {
            ++cpt;
            QTreeWidgetItem * item = new QTreeWidgetItem(this -> ContactsTreeList);
            item -> setText(0, (this -> sortContact -> currentIndex() == 1 ? ((*it) -> last + " " + (*it) -> first) : ((*it) -> first + " " + (*it) -> last)));
            item -> setIcon(0, (*it) -> thumbnail);
            item -> setFont(0, QFont("", 8, QFont::Bold));
            item -> setText(1, (*it) -> nick);
            item -> setText(2, QString::number((*it) -> peopleID));
        }

        this -> ContactsTreeList -> setHeaderLabel(QString::number(cpt) + " " + (cpt > 1 ? tr("contacts") : tr("contact")));
        this -> ContactsTreeList -> sortItems(0, Qt::AscendingOrder);
        this -> showContact(this -> ContactsTreeList -> topLevelItem(0));
        this -> change();
        this -> searchContact -> setEnabled(true);
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

private slots:

    void emitSignal(const QUrl & url) {
        this -> emit linkClicked(url.toString());
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    void updateOptions() {
        if(this -> currentContactDisplayedSet)
            this -> showContact(this -> currentContactDisplayed);
        else this -> makeDefaultWebPage();

        this -> group -> start();
        this -> change();
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    void findContacts(const QString & name) {
        this -> ContactsTreeList -> clear();
        int cpt(0);

        for(std::vector<people *>::iterator it(this -> DBManager -> book.begin()); it != this -> DBManager -> book.end(); ++it) {
            if(((*it) -> first + " " + (*it) -> last).contains(name, Qt::CaseInsensitive)) {
                ++cpt;
                QTreeWidgetItem * item = new QTreeWidgetItem(this -> ContactsTreeList);
                item -> setText(0, (this -> sortContact -> currentIndex() == 1 ? ((*it) -> last + " " + (*it) -> first) : ((*it) -> first + " " + (*it) -> last)));
                item -> setIcon(0, (*it) -> thumbnail);
                item -> setFont(0, QFont("", 8, QFont::Bold));
                item -> setText(1, (*it) -> nick);
                item -> setText(2, QString::number((*it) -> peopleID));
            }
        }

        this -> ContactsTreeList -> setHeaderLabel(QString::number(cpt) + " " + (cpt > 1 ? tr("contacts") : tr("contact")));
        this -> ContactsTreeList -> sortItems(0, Qt::AscendingOrder);
        this -> searchContact -> noResult(cpt == 0);
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    void showContact(QTreeWidgetItem * item, const int & = 0) {
        QString html(""), css(this -> getCSS());

        this -> currentContactDisplayedSet = true;
        this -> currentContactDisplayed = item;

        item -> icon(0).pixmap(50, 50).save(this -> tmpDirectory + "/pic_contacts.png");
        if(!item -> text(1).isEmpty()) css.replace("padding-top: 12px;", "padding-top: 0px;");

        this -> isSMS(item -> text(2));

        html = "<!DOCTYPE HTML><html><head><style type=\"text/css\"><!--" + css +
                "--></style></head><body><div id=\"header\"><b><font size=\"6\">Infos</font></b>" + this -> getDisplayIcon(item -> text(2)) + "</div>"
                "<br/><br/><br/><a href=\"pic/"+ item -> text(2) + "\"><img class=\"pic\" src=\"pic_contacts.png\" width=\"50\" height=\"50\"/></a>"
                "<div id=\"name\">" + item -> text(0) + (item -> text(1).isEmpty() ? "" : "<br/><span id=\"surname\">&laquo; " + item -> text(1) + " &raquo;</span>") +
                "</div><br/><br/>" + this -> analyze(item -> text(2)) + "</body></html>";

        QFile file(this -> tmpDirectory + "/contacts.html");
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream.setCodec("UTF-8");
            file.write(html.toAscii());
            file.close();
        }
        this -> ContactsTreeList -> clearSelection();
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    void change() {
        this -> ContactsWebView -> load(QUrl(this -> tmpDirectory + "/contacts.html"));
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

private:

    void initAttributes(database * db, const QString & path, Options * opt) {
        this -> appPath                    = path;
        this -> tmpDirectory               = this -> appPath + "/utils/tmp";
        this -> DBManager                  = db;
        this -> appOptions                 = opt;
        this -> currentContactDisplayedSet = false;
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    void makeDefaultWebPage() {
        QString html = "<!DOCTYPE HTML><html><head><style type=\"text/css\"><!--" + this -> getCSS() +
                "--></style></head><body><html><div id=\"header\"><b><font size=\"6\">"+ tr("No contact to display") + "</font></b><br/></body></html>";

        QFile index(this -> tmpDirectory + "/contacts.html", this);
        if(index.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&index);
            stream.setCodec("UTF-8");
            index.write(html.toAscii());
            index.close();
        } else qDebug() << tr("Cannot open contacts.html !");
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    QString getCSS() {
        QString content(""), body("../icons/body.png"), header("../icons/header.png"), row("gray"), name("white");
        QFile css(this -> appPath + "/utils/contacts_design.css", this);
        if(css.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&css);
            while(!stream.atEnd())
                content += stream.readLine();
            css.close();

            if(this -> appOptions -> title == "Darkness") {
                header = "../icons/header_black.png";
                body   = "../icons/body_black.png";
                row    = "black";
                name   = "black; color: white";
            }

            content.replace("--body--", body).replace("--header--", header).replace("--name--", name).replace("--row--", row);
        } else qDebug() << tr("Cannot open contacts_design.css !");
        return content;
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    QString analyze(const QString & ID) {
        QString data("");
        people * tmp(this -> getContactFromID(ID));

        for(std::vector<QString>::iterator it(tmp -> cell.begin()); it != tmp -> cell.end(); ++it)
            data += "<div id=\"row\"><span class=\"title\">" + tr("mobile") + "</span><b>" + this -> getCastNum(*it) + "</b></div><br/>";

        for(std::vector<QString>::iterator it(tmp -> home.begin()); it != tmp -> home.end(); ++it)
            data += "<div id=\"row\"><span class=\"title\">" + tr("home") + "</span><b>" + this -> getCastNum(*it) + "</b></div><br/>";

        for(std::multimap<QString, QString>::iterator it(tmp -> infos.begin()); it != tmp -> infos.end(); ++it)
            data += "<div id=\"row\"><span class=\"title\">" + it -> first + "</span><b>" + this -> convert(it -> second) + "</b></div><br/>";

        if(!tmp -> address.isEmpty())
            data += "<div id=\"row\"><span class=\"title\">" + tr("address") + "</span><b>" + tmp -> address.replace("\n", "<br />") + "</b></div><br/>";

        if(!tmp -> homePage.isEmpty())
            data += "<div id=\"row\"><span class=\"title\">" + tr("site") + "</span><b>" + tmp -> homePage + "</b></div><br/>";

        if(!tmp -> work.isEmpty())
            data += "<div id=\"row\"><span class=\"title\">" + tr("work") + "</span><b>" + tmp -> work + "</b></div><br/>";

        return data;
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    QString convert(const QString & data) {
        bool ok(true);
        qint64 data_int(data.toDouble(&ok) * 1000);
        if(ok && data_int > 0) {
            return QDateTime::fromMSecsSinceEpoch(data_int).addYears(31).toString("dd MMMM yyyy");
        } else return data;
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    people * getContactFromID(const QString & ID) {
        for(std::vector<people *>::iterator it(this -> DBManager -> book.begin()); it != this -> DBManager -> book.end(); ++it)
            if(QString::number((*it) -> peopleID) == ID) return *it;
        return 0;
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    QString getCastNum(QString req) {
        QString format("");
        req.replace(" ", "");
        req.replace("+33", "0");
        for(int i(0); i < 5; ++i) {
            format += req.left(2) + " ";
            req.remove(0, 2);
        }
        format.chop(1);
        return format;
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    void isSMS(const QString & ID) {
        emit this -> conversation(ID);
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    QString getDisplayIcon(const QString & ID) {
        if(this -> display)
            return "<a href=\"sms/" + ID + "\" id=\"info\"><img src=\"../icons/sms.png\" title=\"" + tr("View sms conversation") + "\"/></a>";
        else return "";
    }

    /** --------------------------------------------------------------------------------------------------------------- **/

    QTreeWidget                * ContactsTreeList;
    QTreeWidgetItem            * currentContactDisplayed;
    QPixmap                      contactPicture;
    QString                      appPath, tmpDirectory;
    QComboBox                  * sortContact;
    QGraphicsView              * view;
    QSequentialAnimationGroup  * group;

    WebView   * ContactsWebView;
    database  * DBManager;
    SearchBar * searchContact;
    Options   * appOptions;

    bool display, currentContactDisplayedSet;

Q_SIGNALS:

    void linkClicked(const QString &);
    void conversation(const QString &);
};

/** --------------------------------------------------------------------------------------------------------------- **/

#endif // CONTACTS_HPP
