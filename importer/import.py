#!/usr/bin/env python3

 # import.py is a part of ProductInventory
 # An inventory system designed for makeup and related things
 #
 # This program is free software: you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation, either version 3 of the License, or
 # (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program.  If not, see <http://www.gnu.org/licenses/>.

import mysql.connector;

categories = ["Primer/Base", "BB/CC", "Foundation",
	      "Contour", "Bronzer", "Concealer",
	      "Setting Spray", "Highlighter", "Blusher",
	      "Eye Primer", "Brow", "Eyeliner",
	      "Eye Shadow", "Cream Eye Shadow", "Lipstick",
	      "Face Powder", "Mascara", "Glitter",
	      "Glitter Glue", "Eyeshadow Palette",
	      "Lip Liner", "Highlighter Palette",
	      "Blush Palette", "Glitter Liner", "Pigment",
	      "Other", "Tests"];

def truncateTables(toDBCursor):
	print("truncating tables");
	toDBCursor.execute("SET FOREIGN_KEY_CHECKS = 0;");
	toDBCursor.execute("TRUNCATE TABLE category");
	toDBCursor.execute("TRUNCATE TABLE product");
	toDBCursor.execute("TRUNCATE TABLE productimage");
	toDBCursor.execute("SET FOREIGN_KEY_CHECKS = 1;");


def createCategories(toDBCursor):
	for i in categories:
		print("creating category: {}".format(i));
		# we can assume no one is inejcting sql queries here...
		toDBCursor.execute("INSERT INTO category (isactive, name) VALUES(1, \"{}\");".format(i));

def importData(fromDBCursor, toDBCursor):
	print("importing data");
	toDBCursor.execute("SET FOREIGN_KEY_CHECKS = 0;");
	toDBCursor.execute("SELECT categoryid, name FROM category;");
	fromDBCursor.execute("SELECT * from products;");
	todb = toDBCursor.fetchall();
	fromdb = fromDBCursor.fetchall();
	# ok so, i no longer care about this script being nice
	toDBCursor.execute("ALTER TABLE product DROP COLUMN IF EXISTS img;");
	toDBCursor.execute("ALTER TABLE product ADD COLUMN img tinytext;");
	for t in todb:
		print("trying {}".format(t[1]));
		for f in fromdb:
			if f[3] == t[1]:
				print("{} == {}".format(f, t));
				try:
					print(f);
					image = f[7].decode().split("\"")[1];
					toDBCursor.execute("INSERT INTO product "
					"(categoryid, comment, brand, color, "
					"dateadded, deleted, img) VALUES "
					"({}, \"{}\", \"{}\", \"{}\", FROM_UNIXTIME({}), {}, \"{}\");"
					.format(t[0], f[6], f[4], f[5], f[1], f[2], image));

				except IndexError:
					print("no image");
					print(t);
					print(f);
					toDBCursor.execute("INSERT INTO product "
						   "(categoryid, comment, brand, color, "
						   "dateadded, deleted) VALUES "
						   "({}, \"{}\", \"{}\", \"{}\", FROM_UNIXTIME({}), {});"
						   .format(t[0], f[6], f[4], f[5], f[1], f[2]));


				# in our current database we only have 1 image per product
				# so a hack like this will work....
				'''
				try:
					image = f[7].decode().split("\"")[1];
					print(image);
					print(t);
					print("adding {} to productid {}".format(image, t[0]));
					toDBCursor.execute("INSERT INTO productimage (productid, imagename, dateadded)"
							   "VALUES({}, \"{}\", FROM_UNIXTIME({}))".format
							   (t[0], image, f[1], f[7]));
				except IndexError:
					print("no image");
				'''

	toDBCursor.execute("SET FOREIGN_KEY_CHECKS = 1;");

def insertImages(toDBCursor):
	toDBCursor.execute("SELECT productid, img, dateadded FROM product;");
	res = toDBCursor.fetchall();
	for t in res:
		if t[1] is not None:
			print(t[2]);
			print("INSERT INTO productimage (productid, imagename, dateadded) "
					    "VALUES ({}, {}, \"{}\");".format(t[0], t[1], t[2]));
			toDBCursor.execute("INSERT INTO productimage (productid, imagename, dateadded) "
					    "VALUES ({}, \"{}\", \"{}\");".format(t[0], t[1], t[2]));

	toDBCursor.execute("ALTER TABLE product DROP COLUMN IF EXISTS img;");

def main():
	resp = input("Running this script will REMOVE ALL DATABASE ENTRIES in the toDB database, continue? (Y/N): ");
	if resp.upper() != 'Y':
		exit();
	fromDB = mysql.connector.connect(
		host="192.168.1.17",
		user="inventory",
		password="inventorypassword"
		);
	fromDBCursor = fromDB.cursor();
	fromDBCursor.execute("USE inventory;");

	toDB = mysql.connector.connect(
		host="localhost",
		user="inventory",
		password="inventorypassword"
		);
	toDBCursor = toDB.cursor();
	toDBCursor.execute("USE ProductInventory;");

	truncateTables(toDBCursor);
	createCategories(toDBCursor);
	
	importData(fromDBCursor, toDBCursor);
	
	insertImages(toDBCursor);

	toDB.commit();


if __name__ == "__main__":
	main();
