#!/usr/bin/env python3

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
	toDBCursor.execute("SET FOREIGN_KEY_CHECKS = 1;");


def createCategories(toDBCursor):
	for i in categories:
		print("creating category: {}".format(i));
		# we can assume no one is inejcting sql queries here...
		toDBCursor.execute("INSERT INTO category (isactive, name) VALUES(1, \"{}\");".format(i));

def importData(fromDBCursor, toDBCursor):
	'''
	how this works:
	get every category from our todatabase
	loop through each product in fromdb
	where fromdb category name matches todb category name, add product
	after, everything not in one of them is added to the "tests" category
	'''
	print("importing data");
	toDBCursor.execute("SET FOREIGN_KEY_CHECKS = 0;");
	toDBCursor.execute("SELECT categoryid, name FROM category;");
	fromDBCursor.execute("SELECT * from products;");
	todb = toDBCursor.fetchall();
	fromdb = fromDBCursor.fetchall();
	for t in todb:
		print("trying {}".format(t[1]));
		for f in fromdb:
			if f[3] == t[1]:
				print("{} == {}".format(f, t));
				toDBCursor.execute("INSERT INTO product "
						   "(categoryid, comment, brand, color, "
						   "dateadded, deleted) VALUES "
						   "({}, \"{}\", \"{}\", \"{}\", FROM_UNIXTIME({}), {});"
						   .format(t[0], f[6], f[4], f[5], f[1], f[2]));
				# in our current database we only have 1 image per product
				# so a hack like this will work....
				try:
					image = f[7].decode().split("\"")[1];
					print(image);
					print("adding {} to productid {}".format(image, t[0]));
					toDBCursor.execute("INSERT INTO productimage "
							   "VALUES({}, \"{}\", FROM_UNIXTIME({}))".format
							   (t[0], image, f[1]));
				except IndexError:
					print("no image");

	toDBCursor.execute("SET FOREIGN_KEY_CHECKS = 1;");
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

	toDB.commit();


if __name__ == "__main__":
	main();
