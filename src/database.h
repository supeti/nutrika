/*
 * nutrika displays nutrient content
 * Copyright (C) 2011  Peter Sulyok
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 */

#include <gtk/gtk.h>

enum
{ FG_CODE, FG_DESCRIPTION, FG_NCOL };
enum
{ FOOD_NDBNO, FOOD_LONG_DESC, FOOD_NCOL };
enum
{ NUTR_NO, NUTR_VAL, NUTR_UNITS, NUTR_DESC, NUTR_SRORD, NUTR_EAR, NUTR_RDA, NUTR_AI, NUTR_UL, NUTR_PD, NUTR_NCOL };
enum
{ PRODUCT_ID, PRODUCT_NAME, PRODUCT_PRICE, PRODUCT_NCOL };
enum
{ ING_NDBNO, ING_DESC, ING_AMOUNT, ING_AMOUNT_OZ, ING_NCOL };
enum
{ PLN_PRODUCT_ID, PLN_PRODUCT_NAME, PLN_QUANTITY, PLN_NCOL };


void db_open ();
void db_close ();

GtkListStore *db_foodgroups ();
GtkListStore *db_foods (const gchar * fdgrp, const gchar * namelike);
GtkTreeStore *db_food_content (const gchar * ndbno, const int lsg, const int age, const double weight);

GtkListStore *db_products (const gchar * namelike);
void db_insert_product (const gchar * name, const double price);
void db_update_product (const gint product, const double price);
void db_delete_product (const gint id);
GtkListStore *db_ingredients (const gint product);
void db_insert_ingredient (const gint product, const char *ndbno, const char *amount);
void db_delete_ingredient (const gint product, const char *ndbno);
void db_update_ingredient (const gint product, const char *ndbno, const char *amount);
GtkTreeStore *db_product_content (const gint product, const int lsg, const int age, const double weight);

GtkListStore *db_plan ();
GtkTreeStore *db_plan_content (const int lsg, const int age, const double weight, const double days);
void db_plan_product (const gint product, const char *quantity);
void db_unplan_product (const gint product);
void db_update_plan (const gint product, const char *quantity);
