/*
 * database.cpp is a part of ProductInventory
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
 * connect to database
 * \return connect success or fail
 */
bool
ProductInventory::dbConnect()
{
	db.setHostName(ui->HostInput->text());
	db.setPort(ui->portInput->value());
	db.setDatabaseName(ui->databaseInput->text());
	db.setUserName(ui->usernameInput->text());
	db.setPassword(ui->passwordInput->text());
	bool status = db.open();
	if (status)
	{
		isConnected = true;
		conStatus = "Connected";
		conColor = "darkgreen";
		statusMessage("", 0);
		ui->connectButton->setText("Disconnect");
		return true;
	}
	qDebug() << "sql connection failed:" << db.lastError().text();
	genericMessageBox(db.lastError().text(), "server error");
	return false;
}

/*!
 * disconnect from database
 * \return disconnect success or fail
 */
bool
ProductInventory::dbDisconnect()
{
	db.close();
	isConnected = false;
	conStatus = "Disconnected";
	conColor = "red";
	statusMessage("", 0);
	ui->connectButton->setText("Connect");
	return true;
}

bool
ProductInventory::getConnectionStatus()
{
	return isConnected;
}

/*!
 * runs a generic query on the database and returns a pointer to a new
 * QSqlQuery which must be deleted
 * \param query the query to run
 * \return
 */
QSqlQuery *
ProductInventory::genericQuery(QString query)
{
	QSqlQuery *q = new QSqlQuery;
	if (!q->exec(query))
	{
		qDebug() << "query execute failed:" << q->lastError().text();
		genericMessageBox(q->lastError().text(), "query error");
		delete q;
		return nullptr;
	}
	return q;

}

/*!
 * populates the user interface elements with data from the database
 */
void
ProductInventory::populateInterface()
{
//	ui->filterCategoryComboBox->clear();
	QSqlQuery *query = genericQuery("SELECT * FROM category ORDER BY name;");
	if (query == nullptr)
	{
		qDebug() << "failed to populate interface";
		return;
	}
	else
	{
		int x = 0;
		int y = 0;
		while (query->next())
		{
			if (x >= 3)
			{
				y++;
				x = 0;
			}

			QString name = query->value(2).toString();
			//ui->filterCategoryComboBox->addItem(name);
			QCheckBox *chk = new QCheckBox;
			chk->setText(query->value(2).toString());
			checkboxes.append(chk);
			ui->filterCategoryGrid->addWidget(chk, y, x);
			x++;
		}
		delete query;
	}

	//query = genericQuery("SELECT brand, color, comment FROM product");
	query = genericQuery("SELECT categoryid, name FROM category");
	if (query == nullptr)
	{
		qDebug() << "failed to populate interface";
		return;
	}
	else
	{
		while (query->next())
		{
			int t = createTable(query->value(1).toString());
			if (t == -1)
			{
				qDebug() << "failed to populate interface";
				delete query;
				return;

			}
			QSqlQuery *inner = genericQuery("SELECT brand, color, comment, DATE_FORMAT(dateAdded, "
							"\"%d-%m-%Y %h:%i %p\") FROM product WHERE deleted=0 AND categoryid="
							+query->value(0).toString()+" ORDER BY brand DESC");
			if (inner == nullptr)
			{
				qDebug() << "failed to populate interface";
				delete query;
				return;
			}
			while (inner->next())
			{
				QList<QString> item;
				item.append(inner->value(0).toString());
				item.append(inner->value(1).toString());
				item.append(inner->value(2).toString());
				item.append("images here");
				item.append(inner->value(3).toString());
				addItemToTable(tables.at(t), &item);
			}
			if (tables.at(t)->rowCount() == 0)
			{
				tables.at(t)->deleteLater();
				tables.removeAt(t);
				labels.at(t)->deleteLater();
				labels.removeAt(t);
			}
			delete inner;
		}
	}
	resizeRows();
	delete query;
}
