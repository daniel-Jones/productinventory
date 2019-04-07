/*
 * productinventory.cpp is a part of ProductInventory
 * An inventory system designed for makeup and related things
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "productinventory.h"
#include "ui_productinventory.h"

/*!
 * constructor
 * \param parent parent
 */
ProductInventory::ProductInventory(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::ProductInventory)
{
	ui->setupUi(this);
	/* database and statusbar init */
	db = QSqlDatabase::addDatabase("QMYSQL");
	conStatus = "Disconnected";
	conColor = "red";
	isConnected = false;
	statusMessage(conStatus, conColor, 0);

	/* make column headers resize to fit display */
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	/* signal/slot connections */
	connect(ui->statusBar, SIGNAL(messageChanged(const QString)), this, SLOT(statusChanged(QString)));

	ui->tableWidget->hide();
	headerLabels.append(QString("Brand"));
	headerLabels.append(QString("Color"));
	headerLabels.append(QString("Comment"));
}

/*!
 * destructor
 */
ProductInventory::~ProductInventory()
{
	clearTables();
	delete ui;
}

/*!
 * write a message to the statusbar
 * \param msg message to display
 * \param color text color
 * \param ms milliseconds to show message (0 = forever until overwritten)
 */
void
ProductInventory::statusMessage(QString msg, QString color, int ms)
{
	ui->statusBar->setStyleSheet("color: " + color);
	ui->statusBar->showMessage(msg, ms);
}

/*!
 * overload for writing a message to the status bar without a color (defaults to black)
 * \param msg message to display
 * \param ms milliseconds to show message (0 = forever until overwritten)
 */
void
ProductInventory::statusMessage(QString msg, int ms)
{
	statusMessage(msg, "black", ms);
}

/*!
 * slot to receive signal when statusbar message changes
 * \param msg new message
 */
void
ProductInventory::statusChanged(QString msg)
{
	/*
	 * if we have an empty message, display the connection status
	 * indefinitely (empty message received when it is cleared)
	 * BUG: called on hover in file menu
	*/
	if (msg == "")
		statusMessage(conStatus, conColor, 0);
}

/*!
 * slot to receive signal when the connect button is clicked
 */
void
ProductInventory::on_connectButton_clicked()
{
	/*
	 * we use the one button for both connect and disconnect,
	 * so we need to keep track of our status in isConnected
	 * change the button appropriately
	 */
	if (getConnectionStatus())
	{
		// TODO log
		if (dbDisconnect())
		{
			qDebug() << "Disconnected from database";
		}
	}
	else
	{
		db.setHostName(ui->HostInput->text());
		db.setPort((ui->portInput->value()));
		db.setUserName(ui->usernameInput->text());
		db.setPassword(ui->passwordInput->text());
		if (dbConnect())
		{
			// TODO log
			qDebug() << "Connected to database";
			clearTables();
			populateInterface();
		}
	}
}

/*!
 * called when window is resized
 * \param event
 */
void
ProductInventory::resizeEvent(QResizeEvent* event)
{
	QMainWindow::resizeEvent(event);
	ui->tableWidget->resizeRowsToContents(); // not needed?
	QList<QTableWidget *>::iterator i;
	for (i = tables.begin(); i != tables.end(); i++)
	{
		(*i)->resizeRowsToContents();
	}
}

void ProductInventory::on_mustHaveImagesCheckBox_clicked()
{
	ui->mustNotHaveImagesCheckBox->setChecked(false);
}

void ProductInventory::on_mustNotHaveImagesCheckBox_clicked()
{
	ui->mustHaveImagesCheckBox->setChecked(false);
}

/*!
 * add a single item (row) to the table
 * \param table	the table object to add the item to
 * \param values QList<QString> with values to add to the new row
 */
void
ProductInventory::addItemToTable(QTableWidget *table, QList<QString> *values)
{
	QList<QString>::iterator i;
	int col = 0;
	int row = ui->tableWidget->rowCount();
	table->insertRow(row);
	for (i = values->begin(); i != values->end(); i++)
	{
		QTableWidgetItem *test1 = new QTableWidgetItem(*i);
		table->setItem(row, col, test1);
		col++;
	}
}

/*!
 * clear the default table and all other added tables
 * by iterating through the tables list
 */
void
ProductInventory::clearTables()
{
	qDebug() << "clearing table and deleting items";
	ui->tableWidget->setRowCount(0); // automagically deletes items
	QList<QTableWidget *>::iterator i;
	for (i = tables.begin(); i != tables.end(); i++)
	{
		(*i)->setRowCount(0);
		(*i)->hide();
		delete (*i);
		tables.erase(i);
	}
	QList<QLabel *>::iterator l;
	for (l = labels.begin(); l != labels.end(); l++)
	{
		(*l)->hide();
		labels.erase(l);
	}
}

/*!
 * create a QTableWidget, set some properties, append to tables list
 * \return index into tables or -1 on error
 */
int
ProductInventory::createTable(QString category)
{
	QTableWidget *table = new QTableWidget;
	if (table == nullptr)
		return -1;
	table->setMinimumHeight(100);
	table->setColumnCount(3);
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	table->setHorizontalHeaderLabels(headerLabels);
	table->verticalHeader()->hide();
	QLabel *label = new QLabel(category);
	if (label == nullptr)
		return -1;
	labels.append(label);
	QFont f( "Arial", 10, QFont::Bold);
	label->setFont(f);
	ui->scrollArea->widget()->layout()->addWidget(label);
	ui->scrollArea->widget()->layout()->addWidget(table);
	tables.append(table);
	return tables.size()-1;
}
