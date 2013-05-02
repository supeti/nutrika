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

#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#define GETTEXT_PACKAGE "gtk20"
#include <glib/gi18n-lib.h>
#include <glib.h>

sqlite3 *db_con;
sqlite3_stmt *db_food_groups_ps;
sqlite3_stmt *db_foods_ps;
sqlite3_stmt *db_food_content_ps;
sqlite3_stmt *db_products_ps;
sqlite3_stmt *db_products_insert_ps;
sqlite3_stmt *db_products_update_ps;
sqlite3_stmt *db_products_delete_ps;
sqlite3_stmt *db_ingredients_ps;
sqlite3_stmt *db_ingredients_insert_ps;
sqlite3_stmt *db_ingredients_delete_ps;
sqlite3_stmt *db_ingredients_update_ps;
sqlite3_stmt *db_product_content_ps;
sqlite3_stmt *db_plan_ps;
sqlite3_stmt *db_plan_content_ps;
sqlite3_stmt *db_plan_insert_ps;
sqlite3_stmt *db_plan_delete_ps;
sqlite3_stmt *db_plan_update_ps;

#define DB_FATAL_X(rc) \
  if(rc) {\
    g_printerr("%s(%d): %s\n",__FILE__,__LINE__,sqlite3_errmsg(db_con));\
    sqlite3_close(db_con);\
    exit(-2);\
  }
#define DB_WARN_X(rc) if(rc) g_printerr("%s(%d): %d %s\n",__FILE__,__LINE__, sqlite3_errcode(db_con), sqlite3_errmsg(db_con));

void
db_open ()
{
  sqlite3_stmt *ps;
  const char *pztail;
  const gchar *home;
  gchar *nutrids, *user;
  GFile *nutridir, *src, *dst;
  GError *gerr;

#ifdef __MINGW32__
  home = g_getenv ("APPDATA");
  nutrids = g_strconcat (home, "\\nutrika", NULL);
#else
  home = g_getenv ("HOME");
  nutrids = g_strconcat (home, "/.nutrika", NULL);
#endif
  nutridir = g_file_new_for_path (nutrids);
  if (FALSE == g_file_query_exists (nutridir, NULL) && FALSE == g_file_make_directory (nutridir, NULL, NULL))
    {
      g_printerr ("failed to create user data directory: %s\n", nutrids);
      exit (-1);
    }
  g_object_unref (nutridir);
  user = g_strconcat (nutrids, "/user.db", NULL);
  g_free (nutrids);
  src = g_file_new_for_path (USER_DB);
  dst = g_file_new_for_path (user);
  if (FALSE == g_file_query_exists (dst, NULL)
      && FALSE == g_file_copy (src, dst, G_FILE_COPY_TARGET_DEFAULT_PERMS, NULL, NULL, NULL, &gerr))
    {
      g_printerr ("failed to copy the user database %s to local directory %s\n", USER_DB, user);
      exit (-1);
    }
  g_object_unref (src);
  g_object_unref (dst);
  DB_FATAL_X (sqlite3_open_v2 (REFERENCE_DB, &db_con, SQLITE_OPEN_READWRITE, NULL));
  DB_FATAL_X (sqlite3_prepare_v2 (db_con, "ATTACH ? AS user;", -1, &ps, &pztail));
  DB_FATAL_X (sqlite3_bind_text (ps, 1, user, -1, SQLITE_TRANSIENT));
  sqlite3_step (ps);
  g_free (user);
  DB_FATAL_X (sqlite3_finalize (ps));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con, "SELECT code,description FROM food_groups ORDER BY description;", -1, &db_food_groups_ps,
	       &pztail));
  DB_FATAL_X (sqlite3_prepare_v2 (db_con,
				  "SELECT ndb_no,long_desc FROM food_desc WHERE fdgrp=? AND long_desc like ?;",
				  -1, &db_foods_ps, &pztail));
  DB_FATAL_X (sqlite3_prepare_v2 (db_con,
				  "SELECT nutr_data.nutr_no,round(nutr_val,dec),units,nutrdesc,sr_order,Round(nutr_val/ear*100,1)||'%',round(nutr_val/rda*100,1)||'%',round(nutr_val/ai*100,1)||'%',round(nutr_val/ul*100,1)||'%',NULL FROM nutr_data JOIN nutr_def USING(nutr_no) LEFT OUTER JOIN (SELECT nutr_no,lsg,min_age,max_age,ear,rda,ai,ul FROM dri UNION SELECT nutr_no,lsg,min_age,max_age,ear*weight,rda*weight,ai*weight,ul*weight FROM driperkg JOIN (SELECT ? as weight)) AS dri USING (nutr_no) JOIN (SELECT ? AS age, ? AS cur_lsg) WHERE ndb_no=? AND (dri.nutr_no IS NULL OR lsg=cur_lsg AND min_age<=age AND age<max_age) ORDER BY sr_order;",
				  -1, &db_food_content_ps, &pztail));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con, "SELECT id,name,price FROM products WHERE name like ?;", -1, &db_products_ps, &pztail));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con, "INSERT INTO products (name,price) VALUES (?, ?);", -1, &db_products_insert_ps, &pztail));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con, "UPDATE products SET price=? WHERE id=?;", -1, &db_products_update_ps, &pztail));
  DB_FATAL_X (sqlite3_prepare_v2 (db_con, "DELETE FROM products WHERE id=?;", -1, &db_products_delete_ps, &pztail));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con,
	       "SELECT ndb_no,long_desc,amount,round(amount/28.34952,3) FROM ingredients JOIN food_desc USING(ndb_no) WHERE product=?;",
	       -1, &db_ingredients_ps, &pztail));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con, "INSERT INTO ingredients (product,ndb_no,amount) VALUES (?,?,?);", -1, &db_ingredients_insert_ps,
	       &pztail));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con, "DELETE FROM ingredients WHERE product=? AND ndb_no=?;", -1, &db_ingredients_delete_ps,
	       &pztail));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con, "UPDATE ingredients SET amount=? WHERE product=? AND ndb_no=?;", -1, &db_ingredients_update_ps,
	       &pztail));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con,
	       "SELECT nutr_data.nutr_no,Sum(nutr_val*amount*0.01),units,nutrdesc,sr_order,Round(Sum(nutr_val*amount/ear),1)||'%',Round(Sum(nutr_val*amount/rda),1)||'%',Round(Sum(nutr_val*amount/ai),1)||'%',Round(Sum(nutr_val*amount/ul),1)||'%',coalesce(Round(price*100/Sum(nutr_val*amount/rda),2),Round(price*100/Sum(nutr_val*amount/ai),2)) FROM nutr_data JOIN nutr_def USING(nutr_no) JOIN ingredients USING(ndb_no) JOIN products ON ingredients.product=products.id LEFT OUTER JOIN (SELECT nutr_no,lsg,min_age,max_age,ear,rda,ai,ul FROM dri UNION SELECT nutr_no,lsg,min_age,max_age,ear*weight,rda*weight,ai*weight,ul*weight FROM driperkg AS a JOIN (SELECT ? as weight)) AS dri USING (nutr_no) JOIN (SELECT ? AS age, ? AS cur_lsg) WHERE products.id=? AND (dri.nutr_no IS NULL OR lsg=cur_lsg AND min_age<=age AND age<max_age) GROUP BY nutr_data.nutr_no,units,nutrdesc,sr_order ORDER BY sr_order;",
	       -1, &db_product_content_ps, &pztail));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con, "SELECT product,name,quantity FROM plan JOIN products ON product=id", -1, &db_plan_ps, &pztail));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con,
	       "SELECT nutr_data.nutr_no,Round(Sum(nutr_val*amount*0.01*quantity),dec),units,nutrdesc,sr_order,Round(Sum(nutr_val*amount*quantity)/(ear*days),1)||'%',Round(Sum(nutr_val*amount*quantity)/(rda*days),1)||'%',Round(Sum(nutr_val*amount*quantity)/(ai*days),1)||'%',Round(Sum(nutr_val*amount*quantity)/(ul*days),1)||'%',coalesce(Round(Sum(price*quantity*100*(rda*days))/Sum(nutr_val*amount*quantity),2),Round(Sum(price*quantity*100*(ai*days))/Sum(nutr_val*amount*quantity),2)) FROM nutr_data JOIN nutr_def USING(nutr_no) JOIN ingredients USING(ndb_no) JOIN products ON ingredients.product=products.id JOIN plan ON products.id=plan.product LEFT OUTER JOIN (SELECT nutr_no,lsg,min_age,max_age,ear,rda,ai,ul FROM dri UNION SELECT nutr_no,lsg,min_age,max_age,ear*weight,rda*weight,ai*weight,ul*weight FROM driperkg AS a JOIN (SELECT ? as weight)) AS dri USING (nutr_no) JOIN (SELECT ? AS age, ? AS cur_lsg, ? AS days) WHERE (dri.nutr_no IS NULL OR lsg=cur_lsg AND min_age<=age AND age<max_age) GROUP BY nutr_data.nutr_no,units,nutrdesc,sr_order ORDER BY sr_order;",
	       -1, &db_plan_content_ps, &pztail));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con, "INSERT INTO plan (product,quantity) VALUES (?,?);", -1, &db_plan_insert_ps, &pztail));
  DB_FATAL_X (sqlite3_prepare_v2 (db_con, "DELETE FROM plan WHERE product=?;", -1, &db_plan_delete_ps, &pztail));
  DB_FATAL_X (sqlite3_prepare_v2
	      (db_con, "UPDATE plan SET quantity=? WHERE product=?;", -1, &db_plan_update_ps, &pztail));
}

void
db_close ()
{
  sqlite3_close (db_con);
}

GtkListStore *
db_foodgroups ()
{
  const unsigned char *code, *desc;
  GtkListStore *ls;
  GtkTreeIter it;

  ls = gtk_list_store_new (FG_NCOL, G_TYPE_STRING, G_TYPE_STRING);
  while (sqlite3_step (db_food_groups_ps) == SQLITE_ROW)
    {
      code = sqlite3_column_text (db_food_groups_ps, FG_CODE);
      desc = sqlite3_column_text (db_food_groups_ps, FG_DESCRIPTION);
      gtk_list_store_append (ls, &it);
      gtk_list_store_set (ls, &it, FG_CODE, code, FG_DESCRIPTION, desc, -1);
    }
  DB_WARN_X (sqlite3_reset (db_food_groups_ps));
  return ls;
}

GtkListStore *
db_foods (const gchar * fdgrp, const gchar * namelike)
{
  gchar *s;
  const unsigned char *ndbno, *desc;
  GtkListStore *ls = NULL;
  GtkTreeIter it;

  s = g_strconcat ("%%", namelike, "%%", NULL);
  DB_FATAL_X (sqlite3_bind_text (db_foods_ps, 1, fdgrp, -1, SQLITE_TRANSIENT));
  DB_FATAL_X (sqlite3_bind_text (db_foods_ps, 2, s, -1, SQLITE_TRANSIENT));
  g_free (s);
  ls = gtk_list_store_new (FOOD_NCOL, G_TYPE_STRING, G_TYPE_STRING);
  while (sqlite3_step (db_foods_ps) == SQLITE_ROW)
    {
      ndbno = sqlite3_column_text (db_foods_ps, FOOD_NDBNO);
      desc = sqlite3_column_text (db_foods_ps, FOOD_LONG_DESC);
      gtk_list_store_append (ls, &it);
      gtk_list_store_set (ls, &it, FOOD_NDBNO, ndbno, FOOD_LONG_DESC, desc, -1);
    }
  DB_WARN_X (sqlite3_reset (db_foods_ps));
  return ls;
}

const gchar const *nutgrps[6] =
  { N_("Proximates"), N_("Minerals"), N_("Vitamines"), N_("Lipids"), N_("Amino acids"), N_("Others") };
const gint const nutgrpi[7] = { 0, 5300, 6300, 9700, 16300, 18200, 32000 };

GtkTreeStore *
db_content_query (sqlite3_stmt * ps)
{
  int j, srord;
  const unsigned char *nutrno, *nutrval, *units, *desc, *ear, *rda, *ai, *ul, *pd;
  GtkTreeStore *ts;
  GtkTreeIter it1, it2;

  ts =
    gtk_tree_store_new (NUTR_NCOL, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  j = 0;
  while (sqlite3_step (ps) == SQLITE_ROW)
    {
      srord = sqlite3_column_int (ps, NUTR_SRORD);
      nutrno = sqlite3_column_text (ps, NUTR_NO);
      nutrval = sqlite3_column_text (ps, NUTR_VAL);
      units = sqlite3_column_text (ps, NUTR_UNITS);
      desc = sqlite3_column_text (ps, NUTR_DESC);
      ear = sqlite3_column_text (ps, NUTR_EAR);
      rda = sqlite3_column_text (ps, NUTR_RDA);
      ai = sqlite3_column_text (ps, NUTR_AI);
      ul = sqlite3_column_text (ps, NUTR_UL);
      pd = sqlite3_column_text (ps, NUTR_PD);
      while (nutgrpi[j] <= srord)
	{
	  gtk_tree_store_append (ts, &it1, NULL);
	  gtk_tree_store_set (ts, &it1, NUTR_DESC, gettext (nutgrps[j++]), -1);
	}
      gtk_tree_store_append (ts, &it2, &it1);
      gtk_tree_store_set (ts, &it2, NUTR_NO, nutrno, NUTR_VAL, nutrval, NUTR_UNITS, units, NUTR_DESC, desc, NUTR_SRORD,
			  srord, NUTR_EAR, ear, NUTR_RDA, rda, NUTR_AI, ai, NUTR_UL, ul, NUTR_PD, pd, -1);
    }
  DB_WARN_X (sqlite3_reset (ps));
  return ts;
}

GtkTreeStore *
db_food_content (const gchar * ndbno, const int lsg, const int age, const double weight)
{
  DB_FATAL_X (sqlite3_bind_double (db_food_content_ps, 1, weight));
  DB_FATAL_X (sqlite3_bind_int (db_food_content_ps, 2, age));
  DB_FATAL_X (sqlite3_bind_int (db_food_content_ps, 3, lsg));
  DB_FATAL_X (sqlite3_bind_text (db_food_content_ps, 4, ndbno, -1, SQLITE_TRANSIENT));
  return db_content_query (db_food_content_ps);
}

GtkListStore *
db_products (const gchar * namelike)
{
  gchar *s;
  int id;
  const unsigned char *name;
  double price;
  GtkListStore *ls = NULL;
  GtkTreeIter it;

  s = g_strconcat ("%%", namelike, "%%", NULL);
  DB_FATAL_X (sqlite3_bind_text (db_products_ps, 1, s, -1, SQLITE_TRANSIENT));
  g_free (s);
  ls = gtk_list_store_new (PRODUCT_NCOL, G_TYPE_INT, G_TYPE_STRING, G_TYPE_DOUBLE);
  while (sqlite3_step (db_products_ps) == SQLITE_ROW)
    {
      id = sqlite3_column_int (db_products_ps, PRODUCT_ID);
      name = sqlite3_column_text (db_products_ps, PRODUCT_NAME);
      price = sqlite3_column_double (db_products_ps, PRODUCT_PRICE);
      gtk_list_store_append (ls, &it);
      gtk_list_store_set (ls, &it, PRODUCT_ID, id, PRODUCT_NAME, name, PRODUCT_PRICE, price, -1);
    }
  DB_WARN_X (sqlite3_reset (db_products_ps));
  return ls;
}

void
db_insert_product (const gchar * name, const double price)
{
  DB_FATAL_X (sqlite3_bind_text (db_products_insert_ps, 1, name, -1, SQLITE_TRANSIENT));
  DB_FATAL_X (sqlite3_bind_double (db_products_insert_ps, 2, price));
  if (sqlite3_step (db_products_insert_ps) == SQLITE_CONSTRAINT)
    sqlite3_reset (db_products_insert_ps);
  else
    DB_WARN_X (sqlite3_reset (db_products_insert_ps));
}

void
db_update_product (const gint product, const double price)
{
  DB_FATAL_X (sqlite3_bind_int (db_products_update_ps, 1, price));
  DB_FATAL_X (sqlite3_bind_int (db_products_update_ps, 2, product));
  sqlite3_step (db_products_update_ps);
  DB_WARN_X (sqlite3_reset (db_products_update_ps));
}

void
db_delete_product (const gint id)
{
  DB_FATAL_X (sqlite3_bind_int (db_products_delete_ps, 1, id));
  sqlite3_step (db_products_delete_ps);
  DB_WARN_X (sqlite3_reset (db_products_delete_ps));
}

GtkListStore *
db_ingredients (const gint product)
{
  const unsigned char *ndbno, *desc, *amount, *amount_oz;
  GtkListStore *ls = NULL;
  GtkTreeIter it;

  DB_FATAL_X (sqlite3_bind_int (db_ingredients_ps, 1, product));
  ls = gtk_list_store_new (ING_NCOL, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  while (sqlite3_step (db_ingredients_ps) == SQLITE_ROW)
    {
      ndbno = sqlite3_column_text (db_ingredients_ps, ING_NDBNO);
      desc = sqlite3_column_text (db_ingredients_ps, ING_DESC);
      amount = sqlite3_column_text (db_ingredients_ps, ING_AMOUNT);
      amount_oz = sqlite3_column_text (db_ingredients_ps, ING_AMOUNT_OZ);
      gtk_list_store_append (ls, &it);
      gtk_list_store_set (ls, &it, ING_NDBNO, ndbno, ING_DESC, desc, ING_AMOUNT, amount, ING_AMOUNT_OZ, amount_oz, -1);
    }
  DB_WARN_X (sqlite3_reset (db_ingredients_ps));
  return ls;
}

void
db_insert_ingredient (const gint product, const char *ndbno, const char *amount)
{
  DB_FATAL_X (sqlite3_bind_int (db_ingredients_insert_ps, 1, product));
  DB_FATAL_X (sqlite3_bind_text (db_ingredients_insert_ps, 2, ndbno, -1, SQLITE_TRANSIENT));
  DB_FATAL_X (sqlite3_bind_text (db_ingredients_insert_ps, 3, amount, -1, SQLITE_TRANSIENT));
  sqlite3_step (db_ingredients_insert_ps);
  DB_WARN_X (sqlite3_reset (db_ingredients_insert_ps));
}

void
db_delete_ingredient (const gint product, const char *ndbno)
{
  DB_FATAL_X (sqlite3_bind_int (db_ingredients_delete_ps, 1, product));
  DB_FATAL_X (sqlite3_bind_text (db_ingredients_delete_ps, 2, ndbno, -1, SQLITE_TRANSIENT));
  sqlite3_step (db_ingredients_delete_ps);
  DB_WARN_X (sqlite3_reset (db_ingredients_delete_ps));
}

void
db_update_ingredient (const gint product, const char *ndbno, const char *amount)
{
  DB_FATAL_X (sqlite3_bind_text (db_ingredients_update_ps, 1, amount, -1, SQLITE_TRANSIENT));
  DB_FATAL_X (sqlite3_bind_int (db_ingredients_update_ps, 2, product));
  DB_FATAL_X (sqlite3_bind_text (db_ingredients_update_ps, 3, ndbno, -1, SQLITE_TRANSIENT));
  sqlite3_step (db_ingredients_update_ps);
  DB_WARN_X (sqlite3_reset (db_ingredients_update_ps));
}

GtkTreeStore *
db_product_content (const gint product, const int lsg, const int age, const double weight)
{
  DB_FATAL_X (sqlite3_bind_double (db_product_content_ps, 1, weight));
  DB_FATAL_X (sqlite3_bind_int (db_product_content_ps, 2, age));
  DB_FATAL_X (sqlite3_bind_int (db_product_content_ps, 3, lsg));
  DB_FATAL_X (sqlite3_bind_int (db_product_content_ps, 4, product));
  return db_content_query (db_product_content_ps);
}

GtkListStore *
db_plan ()
{
  int product_id;
  const unsigned char *product_name, *quantity;
  GtkListStore *ls = NULL;
  GtkTreeIter it;

  ls = gtk_list_store_new (PLN_NCOL, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);
  while (sqlite3_step (db_plan_ps) == SQLITE_ROW)
    {
      product_id = sqlite3_column_int (db_plan_ps, PLN_PRODUCT_ID);
      product_name = sqlite3_column_text (db_plan_ps, PLN_PRODUCT_NAME);
      quantity = sqlite3_column_text (db_plan_ps, PLN_QUANTITY);
      gtk_list_store_append (ls, &it);
      gtk_list_store_set (ls, &it, PLN_PRODUCT_ID, product_id, PLN_PRODUCT_NAME, product_name, PLN_QUANTITY, quantity,
			  -1);
    }
  DB_WARN_X (sqlite3_reset (db_plan_ps));

  return ls;
}

GtkTreeStore *
db_plan_content (const int lsg, const int age, const double weight, const double days)
{
  DB_FATAL_X (sqlite3_bind_double (db_plan_content_ps, 1, weight));
  DB_FATAL_X (sqlite3_bind_int (db_plan_content_ps, 2, age));
  DB_FATAL_X (sqlite3_bind_int (db_plan_content_ps, 3, lsg));
  DB_FATAL_X (sqlite3_bind_double (db_plan_content_ps, 4, days));
  return db_content_query (db_plan_content_ps);
}

void
db_plan_product (const gint product, const char *quantity)
{
  DB_FATAL_X (sqlite3_bind_int (db_plan_insert_ps, 1, product));
  DB_FATAL_X (sqlite3_bind_text (db_plan_insert_ps, 2, quantity, -1, SQLITE_TRANSIENT));
  sqlite3_step (db_plan_insert_ps);
  DB_WARN_X (sqlite3_reset (db_plan_insert_ps));
}

void
db_unplan_product (const gint product)
{
  DB_FATAL_X (sqlite3_bind_int (db_plan_delete_ps, 1, product));
  sqlite3_step (db_plan_delete_ps);
  DB_WARN_X (sqlite3_reset (db_plan_delete_ps));
}

void
db_update_plan (const gint product, const char *quantity)
{
  DB_FATAL_X (sqlite3_bind_text (db_plan_update_ps, 1, quantity, -1, SQLITE_TRANSIENT));
  DB_FATAL_X (sqlite3_bind_int (db_plan_update_ps, 2, product)) sqlite3_step (db_plan_update_ps);
  DB_WARN_X (sqlite3_reset (db_plan_update_ps));
}
