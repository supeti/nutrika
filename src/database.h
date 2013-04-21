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
{ NUTR_NO, NUTR_VAL, NUTR_UNITS, NUTR_DESC, NUTR_SRORD, NUTR_EAR, NUTR_RDA, NUTR_AI, NUTR_UL, NUTR_NCOL };
enum
{ MEAL_ID, MEAL_NAME, MEAL_NCOL };
enum
{ ING_NDBNO, ING_DESC, ING_AMOUNT, ING_AMOUNT_OZ, ING_NCOL };
enum
{ SCH_DT, SCH_MEAL_ID, SCH_MEAL_NAME, SCH_QUANTITY, SCH_NCOL };

void db_open ();
void db_close ();

GtkListStore *db_foodgroups ();
GtkListStore *db_foods (const gchar * fdgrp, const gchar * namelike);
GtkTreeStore *db_food_content (const gchar * ndbno, const int lsg, const int age, const double weight);

GtkListStore *db_meals (const gchar * namelike);
void db_insert_meal (const gchar * name);
void db_delete_meal (const gint id);
GtkListStore *db_ingredients (const gint meal);
void db_insert_ingredient (const gint meal, const char *ndbno, const char *amount);
void db_delete_ingredient (const gint meal, const char *ndbno);
void db_update_ingredient (const gint meal, const char *ndbno, const char *amount);
GtkTreeStore *db_meal_content (const gint meal, const int lsg, const int age, const double weight);

GtkListStore *db_schedule (const gchar * begin, const gchar * end);
GtkTreeStore *db_schedule_content (const gchar * begin, const gchar * end, const int lsg, const int age,
				   const double weight, const double timespan);
void db_schedule_meal (const char *dt, const gint meal, const char *quantity);
void db_unschedule_meal (const char *dt, const gint meal);
void db_update_schedule (const char *dt, const gint meal, const char *quantity);
