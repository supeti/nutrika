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

#define GETTEXT_PACKAGE "gtk20"
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include "database.h"

const gdouble kgperlb = 0.45359237;
const gdouble gperoz = 28.34952;
gchar *gui_foodgroupcode, *gui_food;
gint gui_meal, gui_lsg = 3, gui_weight_unit = 1;
gdouble gui_sch_timespan, gui_age = 33.0, gui_weight = 70.0;
GDateTime *gui_sch_begin_dt, *gui_sch_end_dt;
GtkWidget *gui_foodnamelike, *gui_food_contents, *gui_mealnamelike, *gui_meal_combo, *gui_newmeal, *gui_ingredients,
  *gui_meal_contents, *gui_schedule, *gui_schedule_cal, *gui_sch_begin, *gui_sch_end, *gui_sch_title,
  *gui_schedule_contents, *gui_schedule_hour, *gui_schedule_minute, *gui_age_spin, *gui_age_unit, *gui_weight_spin;
gchar *gui_sch_begin_str, *gui_sch_end_str;
const gchar *gui_dtf = "%Y-%m-%d %H:%M";
const gchar *gui_dtf0 = "%Y-%m-%d 00:00";

static void
quit (GtkWidget * widget, gpointer data)
{
  gtk_main_quit ();
}

void
foodgroup_changed (GtkComboBox * combo, gpointer user_data)
{
  GtkTreeIter iter;

  g_free (gui_foodgroupcode);
  gtk_combo_box_get_active_iter (combo, &iter);
  gtk_tree_model_get (gtk_combo_box_get_model (combo), &iter, FG_CODE, &gui_foodgroupcode, -1);
}

void
update_tree_view_model (GtkTreeView * tw, GtkTreeModel * tm)
{
  gtk_tree_view_set_model (tw, tm);
  g_object_unref (G_OBJECT (tm));
  gtk_tree_view_expand_all (tw);
}

gboolean
update_combo_model (GtkComboBox * c, GtkTreeModel * tm)
{
  GtkTreeIter iter;

  gtk_combo_box_set_model (c, tm);
  if (gtk_tree_model_get_iter_first (tm, &iter) == TRUE)
    {
      gtk_combo_box_set_active_iter (c, &iter);
      return TRUE;
    }
  else
    return FALSE;
}

void
food_changed (GtkComboBox * combo, gpointer user_data)
{
  GtkTreeIter iter;

  g_free (gui_food);
  gtk_combo_box_get_active_iter (combo, &iter);
  gtk_tree_model_get (gtk_combo_box_get_model (combo), &iter, FOOD_NDBNO, &gui_food, -1);
  update_tree_view_model (GTK_TREE_VIEW (user_data),
			  GTK_TREE_MODEL (db_food_content (gui_food, gui_lsg, gui_age, gui_weight)));
}

void
select_food (GtkButton * widget, gpointer user_data)
{
  if (!update_combo_model
      (GTK_COMBO_BOX (user_data),
       GTK_TREE_MODEL (db_foods (gui_foodgroupcode, gtk_entry_get_text (GTK_ENTRY (gui_foodnamelike))))))
    gtk_tree_view_set_model (GTK_TREE_VIEW (gui_food_contents), NULL);
}

void
select_meal (GtkButton * widget, gpointer user_data)
{
  if (!update_combo_model
      (GTK_COMBO_BOX (gui_meal_combo), GTK_TREE_MODEL (db_meals (gtk_entry_get_text (GTK_ENTRY (gui_mealnamelike))))))
    {
      gtk_tree_view_set_model (GTK_TREE_VIEW (gui_ingredients), NULL);
      gtk_tree_view_set_model (GTK_TREE_VIEW (gui_meal_contents), NULL);
    }
}

void
add_meal (GtkButton * widget, gpointer user_data)
{
  GtkTreeModel *tm;
  GtkTreeIter iter;
  gboolean valid;
  const gchar *newmeal;
  gchar *meal;

  newmeal = gtk_entry_get_text (GTK_ENTRY (gui_newmeal));
  db_insert_meal (newmeal);
  gtk_entry_set_text (GTK_ENTRY (gui_mealnamelike), gtk_entry_get_text (GTK_ENTRY (gui_newmeal)));
  select_meal (NULL, NULL);
  tm = gtk_combo_box_get_model (GTK_COMBO_BOX (gui_meal_combo));
  valid = gtk_tree_model_get_iter_first (tm, &iter);
  while (valid)
    {
      gtk_tree_model_get (tm, &iter, MEAL_NAME, &meal, -1);
      if (g_strcmp0 (newmeal, meal) == 0)
	{
	  gtk_combo_box_set_active_iter (GTK_COMBO_BOX (gui_meal_combo), &iter);
	  valid = FALSE;
	}
      else
	valid = gtk_tree_model_iter_next (tm, &iter);
      g_free (meal);
    }
}

void
delete_meal (GtkButton * widget, gpointer user_data)
{
  GtkTreeModel *tm;
  GtkTreeIter iter;
  gint id;

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (gui_meal_combo), &iter))
    {
      tm = gtk_combo_box_get_model (GTK_COMBO_BOX (gui_meal_combo));
      gtk_tree_model_get (tm, &iter, MEAL_ID, &id, -1);
      db_delete_meal (id);
      select_meal (NULL, NULL);
    }
}

void
meal_changed (GtkComboBox * combo, gpointer user_data)
{
  GtkTreeIter iter;

  gtk_combo_box_get_active_iter (combo, &iter);
  gtk_tree_model_get (gtk_combo_box_get_model (combo), &iter, MEAL_ID, &gui_meal, -1);
  update_tree_view_model (GTK_TREE_VIEW (gui_ingredients), GTK_TREE_MODEL (db_ingredients (gui_meal)));
  update_tree_view_model (GTK_TREE_VIEW (gui_meal_contents),
			  GTK_TREE_MODEL (db_meal_content (gui_meal, gui_lsg, gui_age, gui_weight)));
}

void
add_food_item (GtkButton * widget, gpointer user_data)
{
  db_insert_ingredient (gui_meal, gui_food, "100");
  meal_changed (GTK_COMBO_BOX (gui_meal_combo), NULL);
}

void
remove_food_item (GtkButton * widget, gpointer user_data)
{
  GtkTreeSelection *sel;
  GtkTreeModel *tm;
  GtkTreeIter iter;
  gchar *ndbno;
  gint id;

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (gui_ingredients));
  if (gtk_tree_selection_get_selected (sel, &tm, &iter))
    {
      gtk_tree_model_get (tm, &iter, ING_NDBNO, &ndbno, -1);
      db_delete_ingredient (gui_meal, ndbno);
    }
  meal_changed (GTK_COMBO_BOX (gui_meal_combo), NULL);
}

void
ingredient_amount_edited (GtkCellRendererText * renderer, gchar * path, gchar * new_text, gpointer user_data)
{
  GtkTreeModel *tm;
  GtkTreeIter it;
  gchar *ndbno;

  tm = gtk_tree_view_get_model (GTK_TREE_VIEW (gui_ingredients));
  if (TRUE == gtk_tree_model_get_iter_from_string (tm, &it, path))
    {
      gtk_tree_model_get (tm, &it, ING_NDBNO, &ndbno, -1);
      db_update_ingredient (gui_meal, ndbno, new_text);
      update_tree_view_model (GTK_TREE_VIEW (gui_ingredients), GTK_TREE_MODEL (db_ingredients (gui_meal)));
      update_tree_view_model (GTK_TREE_VIEW (gui_meal_contents),
			      GTK_TREE_MODEL (db_meal_content (gui_meal, gui_lsg, gui_age, gui_weight)));
      g_free (ndbno);
    }
}

void
set_schedule_begin (GtkButton * widget, gpointer user_data)
{
  guint year, month, day, hour, minute;

  gtk_calendar_get_date (GTK_CALENDAR (gui_schedule_cal), &year, &month, &day);
  hour = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (gui_schedule_hour));
  minute = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (gui_schedule_minute));
  g_free (gui_sch_begin_str);
  gui_sch_begin_str = g_strdup_printf ("%.4d-%.2d-%.2d %.2d:%.2d", year, month + 1, day, hour, minute);
  gtk_label_set_text (GTK_LABEL (gui_sch_begin), gui_sch_begin_str);
  g_date_time_unref (gui_sch_begin_dt);
  gui_sch_begin_dt = g_date_time_new_local (year, month + 1, day, hour, minute, 0);
}

void
set_schedule_end (GtkButton * widget, gpointer user_data)
{
  guint year, month, day, hour, minute;

  gtk_calendar_get_date (GTK_CALENDAR (gui_schedule_cal), &year, &month, &day);
  hour = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (gui_schedule_hour));
  minute = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (gui_schedule_minute));
  g_free (gui_sch_end_str);
  gui_sch_end_str = g_strdup_printf ("%.4d-%.2d-%.2d %.2d:%.2d", year, month + 1, day, hour, minute);
  gtk_label_set_text (GTK_LABEL (gui_sch_end), gui_sch_end_str);
  g_date_time_unref (gui_sch_end_dt);
  gui_sch_end_dt = g_date_time_new_local (year, month + 1, day, hour, minute, 0);
}

void
set_schedule_period (GtkButton * widget, gpointer user_data)
{

  gchar *title = g_strdup_printf (_("Schedule from %s to %s"), gui_sch_begin_str, gui_sch_end_str);
  gtk_label_set_text (GTK_LABEL (gui_sch_title), title);
  g_free (title);
  gui_sch_timespan = (double) g_date_time_difference (gui_sch_end_dt, gui_sch_begin_dt) / G_TIME_SPAN_DAY;
  update_tree_view_model (GTK_TREE_VIEW (gui_schedule),
			  GTK_TREE_MODEL (db_schedule (gui_sch_begin_str, gui_sch_end_str)));
  update_tree_view_model (GTK_TREE_VIEW (gui_schedule_contents),
			  GTK_TREE_MODEL (db_schedule_content
					  (gui_sch_begin_str, gui_sch_end_str, gui_lsg, gui_age, gui_weight,
					   gui_sch_timespan)));
}

void
schedule_meal (GtkButton * widget, gpointer user_data)
{
  GtkTreeModel *tm;
  GtkTreeIter iter;
  gint meal;
  guint year, month, day, hour, minute;
  gchar *datetime;

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (gui_meal_combo), &iter))
    {
      tm = gtk_combo_box_get_model (GTK_COMBO_BOX (gui_meal_combo));
      gtk_tree_model_get (tm, &iter, MEAL_ID, &meal, -1);
      gtk_calendar_get_date (GTK_CALENDAR (gui_schedule_cal), &year, &month, &day);
      hour = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (gui_schedule_hour));
      minute = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (gui_schedule_minute));
      datetime = g_strdup_printf ("%.4d-%.2d-%.2d %.2d:%.2d", year, month + 1, day, hour, minute);
      db_schedule_meal (datetime, meal, "1");
      g_free (datetime);
    }
  update_tree_view_model (GTK_TREE_VIEW (gui_schedule),
			  GTK_TREE_MODEL (db_schedule (gui_sch_begin_str, gui_sch_end_str)));
  update_tree_view_model (GTK_TREE_VIEW (gui_schedule_contents),
			  GTK_TREE_MODEL (db_schedule_content
					  (gui_sch_begin_str, gui_sch_end_str, gui_lsg, gui_age, gui_weight,
					   gui_sch_timespan)));
}

void
unschedule_meal (GtkButton * widget, gpointer user_data)
{
  GtkTreeSelection *sel;
  GtkTreeModel *tm;
  GtkTreeIter iter;
  gint meal;
  guint year, month, day, hour, minute;
  gchar *datetime;

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (gui_schedule));
  if (gtk_tree_selection_get_selected (sel, &tm, &iter))
    {
      gtk_tree_model_get (tm, &iter, SCH_DT, &datetime, SCH_MEAL_ID, &meal, -1);

      gtk_calendar_get_date (GTK_CALENDAR (gui_schedule_cal), &year, &month, &day);
      db_unschedule_meal (datetime, meal);
      g_free (datetime);
    }
  update_tree_view_model (GTK_TREE_VIEW (gui_schedule),
			  GTK_TREE_MODEL (db_schedule (gui_sch_begin_str, gui_sch_end_str)));
  update_tree_view_model (GTK_TREE_VIEW (gui_schedule_contents),
			  GTK_TREE_MODEL (db_schedule_content
					  (gui_sch_begin_str, gui_sch_end_str, gui_lsg, gui_age, gui_weight,
					   gui_sch_timespan)));
}

void
schedule_quantity_edited (GtkCellRendererText * renderer, gchar * path, gchar * new_text, gpointer user_data)
{
  GtkTreeModel *tm;
  GtkTreeIter it;
  gchar *dt;
  gint meal_id;

  tm = gtk_tree_view_get_model (GTK_TREE_VIEW (gui_schedule));
  if (TRUE == gtk_tree_model_get_iter_from_string (tm, &it, path))
    {
      gtk_tree_model_get (tm, &it, SCH_DT, &dt, SCH_MEAL_ID, &meal_id, -1);
      db_update_schedule (dt, meal_id, new_text);
      update_tree_view_model (GTK_TREE_VIEW (gui_schedule),
			      GTK_TREE_MODEL (db_schedule (gui_sch_begin_str, gui_sch_end_str)));
      update_tree_view_model (GTK_TREE_VIEW (gui_schedule_contents),
			      GTK_TREE_MODEL (db_schedule_content
					      (gui_sch_begin_str, gui_sch_end_str, gui_lsg, gui_age, gui_weight,
					       gui_sch_timespan)));
      g_free (dt);
    }
}

void
lsg_changed (GtkComboBox * combo, gpointer user_data)
{
  gint old_lsg = gui_lsg;
  gui_lsg = gtk_combo_box_get_active (combo) + 1;
  if (gui_lsg != old_lsg)
    if (1 == gui_lsg)
      gtk_label_set_text (GTK_LABEL (gui_age_unit), _("month"));
    else 
      gtk_label_set_text (GTK_LABEL (gui_age_unit), _("year"));
  switch (gui_lsg)
    {
    case 1:
      gtk_spin_button_set_range (GTK_SPIN_BUTTON (gui_age_spin), 0.0, 11.9);
      break;
    case 2:
      gtk_spin_button_set_range (GTK_SPIN_BUTTON (gui_age_spin), 1.0, 8.9);
      break;
    case 3:
    case 4:
      gtk_spin_button_set_range (GTK_SPIN_BUTTON (gui_age_spin), 9.0, 120.0);
      break;
    case 5:
    case 6:
      gtk_spin_button_set_range (GTK_SPIN_BUTTON (gui_age_spin), 14.0, 50.9);
      break;
    }
}

void
age_changed (GtkSpinButton * spin, gpointer user_data)
{
  gui_age = gtk_spin_button_get_value (spin);
}

void
weight_changed (GtkSpinButton * spin, gpointer user_data)
{
  switch (gui_weight_unit)
    {
    case 1:
      gui_weight = gtk_spin_button_get_value (spin);
      break;
    case 2:
      gui_weight = gtk_spin_button_get_value (spin) * kgperlb;
      break;
    }
}

void
weight_unit_changed (GtkComboBox * combo, gpointer user_data)
{
  gint old_unit = gui_weight_unit;
  gui_weight_unit = gtk_combo_box_get_active (combo) + 1;
  if (gui_weight_unit != old_unit)
  switch (gui_weight_unit)
    {
    case 1:
      gtk_spin_button_set_range (GTK_SPIN_BUTTON (gui_weight_spin), 0.0, 200.0);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (gui_weight_spin), gui_weight);
      break;
    case 2:
      gtk_spin_button_set_range (GTK_SPIN_BUTTON (gui_weight_spin), 0.0, 200.0/kgperlb);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (gui_weight_spin), gui_weight/kgperlb);
      break;
    }
}

GtkWidget *
nutree (GtkWidget ** tree)
{
  GtkCellRenderer *cell;
  GtkWidget *sw;

  sw = gtk_scrolled_window_new (NULL, NULL);
  *tree = gtk_tree_view_new ();
  gtk_widget_set_size_request (*tree, 300, 400);
  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_append_column (GTK_TREE_VIEW (*tree),
			       gtk_tree_view_column_new_with_attributes (_("Value"), cell, "text", NUTR_VAL, NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (*tree),
			       gtk_tree_view_column_new_with_attributes (_("Unit"), cell, "text", NUTR_UNITS, NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (*tree),
			       gtk_tree_view_column_new_with_attributes (_("Description"), cell, "text", NUTR_DESC, NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (*tree),
			       gtk_tree_view_column_new_with_attributes (_("EAR"), cell, "text", NUTR_EAR, NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (*tree),
			       gtk_tree_view_column_new_with_attributes (_("RDA"), cell, "text", NUTR_RDA, NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (*tree),
			       gtk_tree_view_column_new_with_attributes (_("AI"), cell, "text", NUTR_AI, NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (*tree),
			       gtk_tree_view_column_new_with_attributes (_("UL"), cell, "text", NUTR_UL, NULL));
  gtk_tree_view_set_grid_lines(GTK_TREE_VIEW (*tree),GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);
  gtk_container_add (GTK_CONTAINER (sw), *tree);
  return sw;
}

GtkWidget *
foodtab ()
{
  GtkWidget *vbox, *hbox, *button, *label, *combo, *sw;
  GtkCellRenderer *cell;
  GtkRequisition req;
  GtkAdjustment *adjustment;

  vbox = gtk_vbox_new (FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (_("Group:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
  combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (db_foodgroups ()));
  g_signal_connect (combo, "changed", G_CALLBACK (foodgroup_changed), NULL);
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), cell, "text", FG_DESCRIPTION);
  gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  hbox = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (_("Name like:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
  gui_foodnamelike = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), gui_foodnamelike, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
  button = gtk_button_new_with_label ("Select food items");
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 2);

  gui_food = NULL;
  hbox = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (_("Item:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
  combo = gtk_combo_box_new ();
  g_signal_connect (button, "clicked", G_CALLBACK (select_food), combo);
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, FALSE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), cell, "text", FOOD_NDBNO);
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, FALSE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), cell, "text", FOOD_LONG_DESC);
  gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  sw = nutree (&gui_food_contents);
  gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 2);
  g_signal_connect (combo, "changed", G_CALLBACK (food_changed), gui_food_contents);

  return vbox;
}

GtkWidget *
mealtab ()
{
  GtkWidget *vbox, *hbox, *vbox1, *hbox1, *label, *button, *sw, *hpaned, *add_ingredient, *remove_ingredient;
  GtkCellRenderer *cell;
  GtkTreeModel *meals;
  GValue editable = { 0 };
  GValue adv = { 0 };
  GtkObject *adj;

  vbox = gtk_vbox_new (FALSE, 0);
  hbox = gtk_hbox_new (FALSE, 0);

  vbox1 = gtk_vbox_new (FALSE, 0);
  hbox1 = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (_("New meal:"));
  gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, FALSE, 2);
  gui_newmeal = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox1), gui_newmeal, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 2);
  button = gtk_button_new_with_label (_("Add meal"));
  g_signal_connect (button, "clicked", G_CALLBACK (add_meal), NULL);
  gtk_box_pack_start (GTK_BOX (vbox1), button, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), vbox1, FALSE, FALSE, 2);

  gtk_box_pack_start (GTK_BOX (hbox), gtk_vseparator_new (), FALSE, FALSE, 2);

  vbox1 = gtk_vbox_new (FALSE, 0);
  hbox1 = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (_("Name like:"));
  gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, FALSE, 2);
  gui_mealnamelike = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox1), gui_mealnamelike, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 2);
  button = gtk_button_new_with_label (_("Select meal"));
  g_signal_connect (button, "clicked", G_CALLBACK (select_meal), NULL);
  gtk_box_pack_start (GTK_BOX (vbox1), button, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), vbox1, FALSE, FALSE, 2);

  gtk_box_pack_start (GTK_BOX (hbox), gtk_vseparator_new (), FALSE, FALSE, 2);

  vbox1 = gtk_vbox_new (FALSE, 0);
  button = gtk_button_new_with_label (_("Delete meal"));
  g_signal_connect (button, "clicked", G_CALLBACK (delete_meal), NULL);
  gtk_box_pack_start (GTK_BOX (vbox1), button, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), vbox1, TRUE, FALSE, 2);

  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  gui_meal = -1;
  hbox = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (_("Name:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
  gui_meal_combo = gtk_combo_box_new ();
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (gui_meal_combo), cell, FALSE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (gui_meal_combo), cell, "text", MEAL_NAME);
  g_signal_connect (gui_meal_combo, "changed", G_CALLBACK (meal_changed), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), gui_meal_combo, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new (), FALSE, FALSE, 2);

  hpaned = gtk_hpaned_new ();
  vbox1 = gtk_vbox_new (FALSE, 0);
  hbox1 = gtk_hbox_new (TRUE, 0);
  button = gtk_button_new_with_label (_("Add food item"));
  g_signal_connect (button, "clicked", G_CALLBACK (add_food_item), NULL);
  gtk_box_pack_start (GTK_BOX (hbox1), button, TRUE, TRUE, 2);
  button = gtk_button_new_with_label (_("Remove food item"));
  g_signal_connect (button, "clicked", G_CALLBACK (remove_food_item), NULL);
  gtk_box_pack_start (GTK_BOX (hbox1), button, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 2);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_size_request (sw, 300, 400);
  gui_ingredients = gtk_tree_view_new ();

  cell = gtk_cell_renderer_spin_new ();
  g_value_init (&editable, G_TYPE_BOOLEAN);
  g_value_set_boolean (&editable, TRUE);
  g_object_set_property (G_OBJECT (cell), "editable", &editable);
  adj = gtk_adjustment_new (0.0, 0.0, 176.37, 0.1, 10.0, 0.0);
  g_value_init (&adv, G_TYPE_OBJECT);
  g_object_set_property (G_OBJECT (cell), "adjustment", &adv);
  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_ingredients),
			       gtk_tree_view_column_new_with_attributes (_("Weight [oz]"), cell, "text", ING_AMOUNT_OZ, NULL));
  cell = gtk_cell_renderer_spin_new ();
  g_object_set_property (G_OBJECT (cell), "editable", &editable);
  adj = gtk_adjustment_new (0.0, 0.0, 5000.0, 1.0, 10.0, 0.0);
  g_value_set_object (&adv, adj);
  g_object_set_property (G_OBJECT (cell), "adjustment", &adv);
  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_ingredients),
			       gtk_tree_view_column_new_with_attributes (_("Weight [g]"), cell, "text", ING_AMOUNT, NULL));
  g_signal_connect (cell, "edited", G_CALLBACK (ingredient_amount_edited), NULL);
  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_ingredients),
			       gtk_tree_view_column_new_with_attributes (_("NDBNo"), cell, "text", ING_NDBNO, NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_ingredients),
			       gtk_tree_view_column_new_with_attributes (_("Desc"), cell, "text", ING_DESC, NULL));

  gtk_container_add (GTK_CONTAINER (sw), gui_ingredients);
  gtk_box_pack_start (GTK_BOX (vbox1), sw, TRUE, TRUE, 2);
  gtk_paned_pack1 (GTK_PANED (hpaned), vbox1, TRUE, TRUE);
  sw = nutree (&gui_meal_contents);
  gtk_paned_pack2 (GTK_PANED (hpaned), sw, TRUE, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 2);

  return vbox;
}

GtkWidget *
scheduletab ()
{
  GtkWidget *vbox, *hbox, *vbox1, *hbox1, *button, *label, *sw, *hpaned, *frame, *table;
  GtkCellRenderer *cell, *cellspin;
  GValue editable = { 0 };
  GValue digits = { 0 };
  GValue adv = { 0 };
  GtkObject *adj;
  GDateTime *now;
  gint year, month, day;
  gchar *title;

  vbox = gtk_vbox_new (FALSE, 0);
  now = g_date_time_new_now_local ();
  year = g_date_time_get_year (now);
  month = g_date_time_get_month (now);
  day = g_date_time_get_day_of_month (now);
  gui_sch_begin_str = g_strdup_printf ("%.4d-%.2d-%.2d 00:00", year, month + 1, day);
  gui_sch_begin_dt = g_date_time_new_local (year, month, day, 0, 0, 0);
  gui_sch_end_dt = g_date_time_add_days (gui_sch_begin_dt, 1);
  year = g_date_time_get_year (gui_sch_end_dt);
  month = g_date_time_get_month (gui_sch_end_dt);
  day = g_date_time_get_day_of_month (gui_sch_end_dt);
  gui_sch_end_str = g_strdup_printf ("%.4d-%.2d-%.2d 00:00", year, month, day);
  gui_sch_title = gtk_label_new (_("Schedule timeframe has not been set yet."));
  gtk_box_pack_start (GTK_BOX (vbox), gui_sch_title, FALSE, FALSE, 2);

  gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new (), FALSE, FALSE, 2);

  hbox = gtk_hbox_new (FALSE, 0);
  vbox1 = gtk_vbox_new (FALSE, 0);
  gui_schedule_cal = gtk_calendar_new ();
  gtk_box_pack_start (GTK_BOX (vbox1), gui_schedule_cal, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), vbox1, FALSE, FALSE, 2);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new (_("Time:")), FALSE, FALSE, 2);

  adj = gtk_adjustment_new (12, 0, 23, 1, 0, 0);
  gui_schedule_hour = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox1), gui_schedule_hour, FALSE, FALSE, 2);

  gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new (":"), FALSE, FALSE, 2);

  adj = gtk_adjustment_new (0, 0, 59, 1, 0, 0);
  gui_schedule_minute = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox1), gui_schedule_minute, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 2);

  frame = gtk_frame_new (_("Schedule timeframe"));
  table = gtk_table_new (3, 3, FALSE);
  button = gtk_button_new_with_label (_("Begin"));
  g_signal_connect (button, "clicked", G_CALLBACK (set_schedule_begin), NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 0, 1);
  gui_sch_begin = gtk_label_new (gui_sch_begin_str);
  gtk_table_attach_defaults (GTK_TABLE (table), gui_sch_begin, 1, 2, 0, 1);
  button = gtk_button_new_with_label (_("End"));
  g_signal_connect (button, "clicked", G_CALLBACK (set_schedule_end), NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 1, 2);
  gui_sch_end = gtk_label_new (gui_sch_end_str);
  gtk_table_attach_defaults (GTK_TABLE (table), gui_sch_end, 1, 2, 1, 2);
  button = gtk_button_new_with_label (_("Set period"));
  g_signal_connect (button, "clicked", G_CALLBACK (set_schedule_period), NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 2, 2, 3);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, FALSE, 2);

  button = gtk_button_new_with_label (_("Add meal"));
  g_signal_connect (button, "clicked", G_CALLBACK (schedule_meal), NULL);
  gtk_box_pack_start (GTK_BOX (vbox1), button, FALSE, FALSE, 2);
  button = gtk_button_new_with_label (_("Remove meal"));
  g_signal_connect (button, "clicked", G_CALLBACK (unschedule_meal), NULL);
  gtk_box_pack_start (GTK_BOX (vbox1), button, FALSE, FALSE, 2);

  gtk_box_pack_start (GTK_BOX (hbox), gtk_vseparator_new (), FALSE, FALSE, 2);

  hpaned = gtk_hpaned_new ();
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_size_request (sw, 300, 400);
  gui_schedule = gtk_tree_view_new ();
  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_schedule),
			       gtk_tree_view_column_new_with_attributes (_("Date&Time"), cell, "text", SCH_DT, NULL));

  cellspin = gtk_cell_renderer_spin_new ();
  g_value_init (&editable, G_TYPE_BOOLEAN);
  g_value_set_boolean (&editable, TRUE);
  g_object_set_property (G_OBJECT (cellspin), "editable", &editable);
  g_value_init (&digits, G_TYPE_INT);
  g_value_set_int (&digits, 1);
  g_object_set_property (G_OBJECT (cellspin), "digits", &digits);
  adj = gtk_adjustment_new (0.0, 0.0, 10.0, 0.1, 1.0, 0.0);
  g_value_init (&adv, G_TYPE_OBJECT);
  g_value_set_object (&adv, adj);
  g_object_set_property (G_OBJECT (cellspin), "adjustment", &adv);
  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_schedule),
			       gtk_tree_view_column_new_with_attributes (_("Quantity"), cellspin, "text", SCH_QUANTITY,
									 NULL));
  g_signal_connect (cellspin, "edited", G_CALLBACK (schedule_quantity_edited), NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_schedule),
			       gtk_tree_view_column_new_with_attributes (_("Meal"), cell, "text", SCH_MEAL_NAME, NULL));

  gtk_container_add (GTK_CONTAINER (sw), gui_schedule);
  gtk_paned_pack1 (GTK_PANED (hpaned), sw, TRUE, TRUE);
  sw = nutree (&gui_schedule_contents);
  gtk_paned_pack2 (GTK_PANED (hpaned), sw, TRUE, TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), hpaned, TRUE, TRUE, 2);



  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 2);



  return vbox;
}

GtkWidget *
eatertab ()
{
  GtkWidget *vbox, *hbox, *vbox1, *hbox1, *combo, *label, *hpaned, *frame, *table;
  gchar *groups[6] = { N_("Infants"), N_("Children"), N_("Males"), N_("Females"), N_("Pregnancy"), N_("Lactation") };
  gchar *weight_units[2] = { N_("Kg"), N_("lb") };
  gint i;

  vbox = gtk_vbox_new (FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (_("Life Stage Group:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
  combo = gtk_combo_box_new_text ();
  for (i = 0; i != 6; i++)
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), gettext(groups[i]));
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 2);
  gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);
  g_signal_connect (combo, "changed", G_CALLBACK (lsg_changed), NULL);

  hbox = gtk_hbox_new (FALSE, 0);
  table = gtk_table_new (3, 3, FALSE);

  label = gtk_label_new (_("Age:"));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);
  gui_age_spin = gtk_spin_button_new_with_range (9.0, 120.0, 1.0);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (gui_age_spin), 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (gui_age_spin), gui_age);
  g_signal_connect (gui_age_spin, "value-changed", G_CALLBACK (age_changed), NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), gui_age_spin, 1, 2, 0, 1);
  gui_age_unit = gtk_label_new (_("year"));
  gtk_table_attach_defaults (GTK_TABLE (table), gui_age_unit, 2, 3, 0, 1);

  hbox = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (_("Weight:"));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);
  gui_weight_spin = gtk_spin_button_new_with_range (0.0, 200.0, 1.0);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (gui_weight_spin), 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (gui_weight_spin), gui_weight);
  g_signal_connect (gui_weight_spin, "value-changed", G_CALLBACK (weight_changed), NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), gui_weight_spin, 1, 2, 1, 2);
  combo = gtk_combo_box_new_text ();
  for (i = 0; i != 2; i++)
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), gettext(weight_units[i]));
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  g_signal_connect (combo, "changed", G_CALLBACK (weight_unit_changed), NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), combo, 2, 3, 1, 2);

  gtk_box_pack_start (GTK_BOX (hbox), table, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  return vbox;
}


GtkWidget *
notebook ()
{
  GtkWidget *notebook;

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), foodtab (), gtk_label_new (_("Food")));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), mealtab (), gtk_label_new (_("Meal")));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scheduletab (), gtk_label_new (_("Schedule")));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), eatertab (), gtk_label_new (_("Eater")));

  return notebook;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *menu_item, *menu_bar;
  GError *error = NULL;

  gtk_init (&argc, &argv);
  db_open ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), _("Nutrika 0.2.1"));
  g_signal_connect (window, "delete-event", G_CALLBACK (quit), NULL);
  g_signal_connect (window, "destroy", G_CALLBACK (quit), NULL);
  gtk_window_set_icon (GTK_WINDOW (window), gdk_pixbuf_new_from_file(NUTRIKA_ICON, &error));

  box = gtk_vbox_new (FALSE, 0);

  gtk_box_pack_start (GTK_BOX (box), notebook (), TRUE, TRUE, 2);

  gtk_container_add (GTK_CONTAINER (window), box);
  gtk_widget_show_all (window);
  gtk_main ();

  db_close ();

  return 0;
}
