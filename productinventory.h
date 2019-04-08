/*
 * productinventory.h a part of ProductInventory
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

#ifndef PRODUCTINVENTORY_H
#define PRODUCTINVENTORY_H

#include <QMainWindow>
#include <QStatusBar>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QSplitter>
#include <QCheckBox>
#include <QGridLayout>
#include <QDebug>

namespace Ui {
class ProductInventory;
}

class ProductInventory : public QMainWindow
{
	Q_OBJECT

public:
	explicit ProductInventory(QWidget *parent = nullptr);
	~ProductInventory();
	bool getConnectionStatus(); 			// get database connection status

	void statusMessage(QString msg, int ms);		// overload, show message in status bar for ms milliseconds
	void statusMessage(QString msg, QString color, int ms); // show message in status bar for ms milliseconds
	void genericMessageBox(QString msg, QString title);	// display a generic message box
	QSqlQuery *genericQuery(QString query);			// run generic query and return a pointer to the new object

private slots:
	void statusChanged(QString msg);		// called when the status bar value is changed
	void on_connectButton_clicked();		// called when the connect button is pressed

	void on_mustHaveImagesCheckBox_clicked();	// called when must have images filter selected
	void on_mustNotHaveImagesCheckBox_clicked();	// called when must not have images filter selected

private:
	Ui::ProductInventory *ui;
	QString conStatus; 				// connection status text
	QString conColor; 				// color of current connection status
	bool isConnected;	 			// store our conection status

	bool dbConnect(); 				// connect to database
	bool dbDisconnect(); 				// disconnect from database
	QSqlDatabase db;				// database object
	void resizeEvent(QResizeEvent *event);		// called when the window is resized
	void populateInterface();			// called when we want to populate interface elements
	void addItemToTable(QTableWidget *table,
			    QList<QString> *values);	// add item to the specified table with the provided values
	void clearTables();				// clear and delete every QTabeWidgetItem in each table
	QList<QTableWidget *> tables;			// store our tables
	int createTable(QString category); 		// create a table and add it to our tables list with a header, returns index in list or -1
	QStringList headerLabels;			// labels used for table headers
	QList<QLabel *> labels;				// labels used for table names
	void resizeRows();				// resize all table rows to fit the content
	QList<QCheckBox *> checkboxes;			// checkboxes used to filter product categories
	QGridLayout *categoryLayout;			// layout
};

#endif // PRODUCTINVENTORY_H
