/*
 * nutrika displays nutrient content
 * Copyright (C) 2011,2013  Peter Sulyok
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

#define GETTEXT_PACKAGE "gtk30"
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include "database.h"

const gdouble kgperlb = 0.45359237;
const gdouble gperoz = 28.34952;
const gchar *inifile;
gchar *gui_foodgroupcode, *gui_food;
gint gui_prod, gui_lsg = 3, gui_weight_unit = 1;
gdouble gui_sch_timespan = 7.0, gui_age = 33.0, gui_weight = 70.0;
GtkWidget *gui_foodnamelike, *gui_foodgroup, *gui_fooditem, *gui_food_contents, *gui_prodnamelike,
  *gui_prod_combo, *gui_newprod, *gui_price_entry, *gui_prod_spinner,
  *gui_ingredients, *gui_prod_contents, *gui_plan, *gui_plan_contents, *gui_age_spin, *gui_age_unit,
  *gui_weight_spin, *gui_plan_days;
GtkTreeIter gui_food_iter, gui_prod_iter, gui_ing_iter;

void init()
{
  GKeyFile *kf;
  GError *error;
  const gchar *home;
  gchar *nutrids;

#ifdef __MINGW32__
  home = g_getenv ("APPDATA");
  nutrids = g_strconcat (home, "\\nutrika", NULL);
#else
  home = g_getenv ("HOME");
  nutrids = g_strconcat (home, "/.nutrika", NULL);
#endif
  inifile = g_strconcat (nutrids, "/nutrika.ini", NULL);
  g_free (nutrids);
  if (g_key_file_load_from_file(kf, inifile, G_KEY_FILE_NONE, &error) == TRUE)
  {
    gui_lsg = g_key_file_get_integer (kf, "settings", "LSG", &error);
    gui_weight_unit = g_key_file_get_integer (kf, "settings", "weightUnit", &error);
    gui_age = g_key_file_get_double  (kf, "settings", "age", &error);
    gui_weight = g_key_file_get_double  (kf, "settings", "weight", &error);
    gui_sch_timespan = g_key_file_get_double  (kf, "settings", "timespan", &error);
  }
}

static void
quit (GtkWidget * widget, gpointer data)
{
  GKeyFile *kf;
  GError *error;
  gsize length;
  const char *buf;
  kf = g_key_file_new ();
  g_key_file_set_integer (kf, "settings", "LSG", gui_lsg);
  g_key_file_set_integer (kf, "settings", "weightUnit", gui_weight_unit);
  g_key_file_set_double (kf, "settings", "age", gui_age);
  g_key_file_set_double (kf, "settings", "weight", gui_weight);
  g_key_file_set_double (kf, "settings", "timespan", gui_sch_timespan);
  buf = g_key_file_to_data(kf, &length, &error);
  g_file_set_contents (inifile, buf, length, &error);
  gtk_main_quit ();
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
  g_object_unref (G_OBJECT (tm));
  if (gtk_tree_model_get_iter_first (tm, &iter) == TRUE)
    {
      gtk_combo_box_set_active_iter (c, &iter);
      return TRUE;
    }
  else
    return FALSE;
}

void update_food()
{
  if (!update_combo_model
      (GTK_COMBO_BOX (gui_fooditem),
       GTK_TREE_MODEL (db_foods (gui_foodgroupcode, gtk_entry_get_text (GTK_ENTRY (gui_foodnamelike))))))
    gtk_tree_view_set_model (GTK_TREE_VIEW (gui_food_contents), NULL);
}

void
foodnamelike (GtkEntry * entry, gpointer user_data)
{
  update_food(NULL, NULL);
}

void
foodgroup_changed (GtkComboBox * combo, gpointer user_data)
{
  GtkTreeIter iter;

  g_free (gui_foodgroupcode);
  gtk_combo_box_get_active_iter (combo, &iter);
  gtk_tree_model_get (gtk_combo_box_get_model (combo), &iter, FG_CODE, &gui_foodgroupcode, -1);
  update_food(NULL, NULL);
}

void
food_changed (GtkComboBox * combo, gpointer user_data)
{
  g_free (gui_food);
  gtk_combo_box_get_active_iter (combo, &gui_food_iter);
  gtk_tree_model_get (gtk_combo_box_get_model (combo), &gui_food_iter, FOOD_NDBNO, &gui_food, -1);
  update_tree_view_model (GTK_TREE_VIEW (user_data),
			  GTK_TREE_MODEL (db_food_content (gui_food, gui_lsg, gui_age, gui_weight)));
}

void
prodnamelike (GtkEntry * entry, gpointer user_data)
{
  double price;
  GtkComboBox *combo;

  combo = GTK_COMBO_BOX (gui_prod_combo);
  if (!update_combo_model
      (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (db_products (gtk_entry_get_text (GTK_ENTRY (gui_prodnamelike))))))
    {
      gtk_tree_view_set_model (GTK_TREE_VIEW (gui_ingredients), NULL);
      gtk_tree_view_set_model (GTK_TREE_VIEW (gui_prod_contents), NULL);
    }
}

void
add_product (GtkEntry * entry, gpointer user_data)
{
  GtkTreeModel *tm;
  GtkTreeIter iter;
  gboolean valid;
  const gchar *newproduct;
  gchar *product;
  double price = 1.0;

  newproduct = gtk_entry_get_text (GTK_ENTRY (gui_newprod));
  db_insert_product (newproduct, price);
  gtk_entry_set_text (GTK_ENTRY (gui_prodnamelike), gtk_entry_get_text (GTK_ENTRY (gui_newprod)));
  prodnamelike (NULL, NULL);
  tm = gtk_combo_box_get_model (GTK_COMBO_BOX (gui_prod_combo));
  valid = gtk_tree_model_get_iter_first (tm, &iter);
  while (valid)
    {
      gtk_tree_model_get (tm, &iter, PRODUCT_NAME, &product, -1);
      if (g_strcmp0 (newproduct, product) == 0)
	{
	  gtk_combo_box_set_active_iter (GTK_COMBO_BOX (gui_prod_combo), &iter);
	  valid = FALSE;
	}
      else
	valid = gtk_tree_model_iter_next (tm, &iter);
      g_free (product);
    }
}

void
delete_product (GtkButton * widget, gpointer user_data)
{
  GtkTreeModel *tm;
  GtkTreeIter iter;
  gint id;

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (gui_prod_combo), &iter))
    {
      tm = gtk_combo_box_get_model (GTK_COMBO_BOX (gui_prod_combo));
      gtk_tree_model_get (tm, &iter, PRODUCT_ID, &id, -1);
      db_delete_product (id);
      prodnamelike (NULL, NULL);
    }
}

void
product_price_changed (GtkEntry * entry, gpointer user_data)
{
  GtkTreeModel *tm;
  GtkTreeIter iter;
  GtkComboBox *combo;
  double price;
  gint id, it;

  price = g_ascii_strtod(gtk_entry_get_text (entry), NULL);
  db_update_product (gui_prod, price);
  combo = GTK_COMBO_BOX (gui_prod_combo);
  gtk_combo_box_get_active_iter (combo, &iter);
  tm = gtk_combo_box_get_model (combo);
  gtk_list_store_set (GTK_LIST_STORE (tm), &iter, PRODUCT_PRICE, price, -1);
  update_tree_view_model (GTK_TREE_VIEW (gui_prod_contents),
			  GTK_TREE_MODEL (db_product_content (gui_prod, gui_lsg, gui_age, gui_weight)));
  update_tree_view_model (GTK_TREE_VIEW (gui_plan_contents),
			  GTK_TREE_MODEL (db_plan_content (gui_lsg, gui_age, gui_weight, gui_sch_timespan)));
}

void
product_changed (GtkComboBox * combo, gpointer user_data)
{
  GtkTreeIter iter;
  double price;
  gchar qg[10];

  gtk_combo_box_get_active_iter (combo, &iter);
  gtk_tree_model_get (gtk_combo_box_get_model (combo), &iter, PRODUCT_ID, &gui_prod, PRODUCT_PRICE, &price, -1);
  gtk_entry_set_text (GTK_ENTRY (gui_price_entry), g_ascii_dtostr(qg,10,price));
  update_tree_view_model (GTK_TREE_VIEW (gui_ingredients), GTK_TREE_MODEL (db_ingredients (gui_prod)));
  update_tree_view_model (GTK_TREE_VIEW (gui_prod_contents),
			  GTK_TREE_MODEL (db_product_content (gui_prod, gui_lsg, gui_age, gui_weight)));
}

void
add_food_item (GtkButton * widget, gpointer user_data)
{
  gchar *desc;
  GtkListStore *ls;
  GtkTreeIter iter;
    
  db_insert_ingredient (gui_prod, gui_food, "100");
  gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX(gui_fooditem)), &gui_food_iter, FOOD_LONG_DESC, &desc, -1);
  ls = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (gui_ingredients)));
  gtk_list_store_append (ls, &iter);
  gtk_list_store_set (ls, &iter, ING_NDBNO, gui_food, ING_DESC, desc, ING_AMOUNT, "100", ING_AMOUNT_OZ, "3.527", -1);
  g_free (desc);
  update_tree_view_model (GTK_TREE_VIEW (gui_prod_contents),
			  GTK_TREE_MODEL (db_product_content (gui_prod, gui_lsg, gui_age, gui_weight)));
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
      db_delete_ingredient (gui_prod, ndbno);
    }
  gtk_list_store_remove(GTK_LIST_STORE(tm), &iter);
  update_tree_view_model (GTK_TREE_VIEW (gui_prod_contents),
			  GTK_TREE_MODEL (db_product_content (gui_prod, gui_lsg, gui_age, gui_weight)));
}

void
ingredient_amount_edited (GtkCellRendererText * renderer, gchar * path, gchar * amount_oz, gchar * amount_g)
{
  GtkTreeModel *tm;
  GtkTreeIter it;
  gchar *ndbno;

  tm = gtk_tree_view_get_model (GTK_TREE_VIEW (gui_ingredients));
  if (TRUE == gtk_tree_model_get_iter_from_string (tm, &it, path))
    {
      gtk_tree_model_get (tm, &it, ING_NDBNO, &ndbno, -1);
      db_update_ingredient (gui_prod, ndbno, amount_g);
      g_free (ndbno);
      gtk_list_store_set (GTK_LIST_STORE(tm), &it, ING_AMOUNT_OZ, amount_oz, ING_AMOUNT, amount_g, -1);
      update_tree_view_model (GTK_TREE_VIEW (gui_prod_contents),
			      GTK_TREE_MODEL (db_product_content (gui_prod, gui_lsg, gui_age, gui_weight)));
    }
}

void
ingredient_amount_edited_g (GtkCellRendererText * renderer, gchar * path, gchar * new_text, gpointer user_data)
{
  gchar qg[10], qoz[10];
  ingredient_amount_edited(renderer, path, g_ascii_dtostr(qoz,10,g_ascii_strtod(new_text, NULL)/28.34952), g_ascii_dtostr(qg,10,g_ascii_strtod(new_text, NULL)));
}

void
ingredient_amount_edited_oz (GtkCellRendererText * renderer, gchar * path, gchar * new_text, gpointer user_data)
{
  gchar qg[10], qoz[10];
  ingredient_amount_edited(renderer, path, g_ascii_dtostr(qoz,10,g_ascii_strtod(new_text, NULL)), g_ascii_dtostr(qg,10,g_ascii_strtod(new_text, NULL)*28.34952)); 
}

void
update_plan_contents ()
{
  update_tree_view_model (GTK_TREE_VIEW (gui_plan), GTK_TREE_MODEL (db_plan ()));
  update_tree_view_model (GTK_TREE_VIEW (gui_plan_contents),
			  GTK_TREE_MODEL (db_plan_content (gui_lsg, gui_age, gui_weight, gui_sch_timespan)));
}

void
plan_product (GtkButton * widget, gpointer user_data)
{
  GtkTreeModel *tm;
  GtkTreeIter iter;
  gint product;
  guint year, month, day, hour, minute;
  gchar *datetime;

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (gui_prod_combo), &iter))
    {
      tm = gtk_combo_box_get_model (GTK_COMBO_BOX (gui_prod_combo));
      gtk_tree_model_get (tm, &iter, PRODUCT_ID, &product, -1);
      db_plan_product (product, "1");
    }
  update_plan_contents ();
}

void
unplan_product (GtkButton * widget, gpointer user_data)
{
  GtkTreeSelection *sel;
  GtkTreeModel *tm;
  GtkTreeIter iter;
  gint product;
  guint year, month, day, hour, minute;
  gchar *datetime;

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (gui_plan));
  if (gtk_tree_selection_get_selected (sel, &tm, &iter))
    {
      gtk_tree_model_get (tm, &iter, PRODUCT_ID, &product, -1);
      db_unplan_product (product);
    }
  update_plan_contents ();
}

void
plan_quantity_edited (GtkCellRendererText * renderer, gchar * path, gchar * new_text, gpointer user_data)
{
  GtkTreeModel *tm;
  GtkTreeIter it;
  gint product_id;
  gchar quantity[10];

  tm = gtk_tree_view_get_model (GTK_TREE_VIEW (gui_plan));
  if (TRUE == gtk_tree_model_get_iter_from_string (tm, &it, path))
    {
      gtk_tree_model_get (tm, &it, PLN_PRODUCT_ID, &product_id, -1);
      db_update_plan (product_id, g_ascii_dtostr(quantity,10,g_ascii_strtod(new_text, NULL)));
      update_plan_contents ();
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
plan_days_changed (GtkSpinButton * spin, gpointer user_data)
{
  gui_sch_timespan = gtk_spin_button_get_value (spin);
  update_tree_view_model (GTK_TREE_VIEW (gui_plan_contents),
			  GTK_TREE_MODEL (db_plan_content (gui_lsg, gui_age, gui_weight, gui_sch_timespan)));
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
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (gui_weight_spin), 0.0, 200.0 / kgperlb);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (gui_weight_spin), gui_weight / kgperlb);
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
			       gtk_tree_view_column_new_with_attributes (_("Description"), cell, "text", NUTR_DESC,
									 NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (*tree),
			       gtk_tree_view_column_new_with_attributes (_("EAR"), cell, "text", NUTR_EAR, NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (*tree),
			       gtk_tree_view_column_new_with_attributes (_("RDA"), cell, "text", NUTR_RDA, NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (*tree),
			       gtk_tree_view_column_new_with_attributes (_("AI"), cell, "text", NUTR_AI, NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (*tree),
			       gtk_tree_view_column_new_with_attributes (_("UL"), cell, "text", NUTR_UL, NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (*tree),
			       gtk_tree_view_column_new_with_attributes (_("P/V"), cell, "text", NUTR_PV, NULL));
  gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (*tree), GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);
  gtk_container_add (GTK_CONTAINER (sw), *tree);
  return sw;
}

GtkWidget *
foodtab ()
{
  GtkWidget *vgrid, *grid, *button, *label, *sw;
  GtkCellRenderer *cell;
  GtkRequisition req;
  GtkAdjustment *adjustment;

  vgrid = gtk_grid_new ();
  grid = gtk_grid_new ();

  label = gtk_label_new (_("Select food items like "));
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
#ifdef __MINGW32__
  gui_foodnamelike = gtk_entry_new ();
#else
  gui_foodnamelike = gtk_search_entry_new ();
#endif
  gtk_widget_set_tooltip_text (gui_foodnamelike,
			       "Type some characters and press Enter to filter food items. If empty, all food items will appear from the selected food group.");
  gtk_grid_attach_next_to (GTK_GRID (grid), gui_foodnamelike, label, GTK_POS_RIGHT, 1, 1);
  label = gtk_label_new (_(" in group: "));
  gtk_grid_attach_next_to (GTK_GRID (grid), label, gui_foodnamelike, GTK_POS_RIGHT, 1, 1);
  gui_foodgroup = gtk_combo_box_new_with_model (GTK_TREE_MODEL (db_foodgroups ()));
  gtk_widget_set_tooltip_text (gui_foodgroup, "Choose a food group here.");
  gtk_combo_box_set_active (GTK_COMBO_BOX (gui_foodgroup), 0);
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (gui_foodgroup), cell, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (gui_foodgroup), cell, "text", FG_DESCRIPTION);
  gtk_grid_attach_next_to (GTK_GRID (grid), gui_foodgroup, label, GTK_POS_RIGHT, 1, 1);
  gtk_grid_attach (GTK_GRID (vgrid), grid, 0, 0, 1, 1);

  grid = gtk_grid_new ();
  gui_food = NULL;
  label = gtk_label_new (_("100g "));
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
  gui_fooditem = gtk_combo_box_new ();
  gtk_widget_set_tooltip_text (gui_fooditem, "Select a food item here to see its nutrient contents below.");
  g_signal_connect (gui_foodnamelike, "activate", G_CALLBACK (foodnamelike), NULL);
  g_signal_connect (gui_foodgroup, "changed", G_CALLBACK (foodgroup_changed), NULL);
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (gui_fooditem), cell, FALSE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (gui_fooditem), cell, "text", FOOD_NDBNO);
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (gui_fooditem), cell, FALSE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (gui_fooditem), cell, "text", FOOD_LONG_DESC);
  gtk_grid_attach_next_to (GTK_GRID (grid), gui_fooditem, label, GTK_POS_RIGHT, 5, 1);
  label = gtk_label_new (_(" contains:"));
  gtk_grid_attach_next_to (GTK_GRID (grid), label, gui_fooditem, GTK_POS_RIGHT, 1, 1);
  gtk_grid_attach_next_to (GTK_GRID (vgrid), grid, NULL, GTK_POS_BOTTOM, 1, 1);

  sw = nutree (&gui_food_contents);
  gtk_widget_set_hexpand (sw, TRUE);
  gtk_widget_set_vexpand (sw, TRUE);
  gtk_widget_set_halign (sw, GTK_ALIGN_FILL);
  gtk_widget_set_valign (sw, GTK_ALIGN_FILL);
  g_signal_connect (gui_fooditem, "changed", G_CALLBACK (food_changed), gui_food_contents);
  gtk_grid_attach_next_to (GTK_GRID (vgrid), sw, NULL, GTK_POS_BOTTOM, 1, 1);

  return vgrid;
}

GtkWidget *
producttab ()
{
  GtkWidget *vgrid, *grid, *grid1, *grid2, *label, *button, *sw, *hpaned, *add_ingredient, *remove_ingredient;
  GtkCellRenderer *cell;
  GtkTreeModel *products;
  GValue editable = { 0 };
  GValue adv = { 0 };
  GtkAdjustment *adj;

  vgrid = gtk_grid_new ();
  grid = gtk_grid_new ();

  label = gtk_label_new (_("Add "));
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
  gui_newprod = gtk_entry_new ();
  gtk_widget_set_tooltip_text (gui_newprod, "Type a name for the new product, and press Enter.");
  g_signal_connect (gui_newprod, "activate", G_CALLBACK (add_product), NULL);
  gtk_grid_attach_next_to (GTK_GRID (grid), gui_newprod, label, GTK_POS_RIGHT, 1, 1);
  label = gtk_label_new (_("Select products like "));
  gtk_grid_attach_next_to (GTK_GRID (grid), label, NULL, GTK_POS_BOTTOM, 1, 1);
#ifdef __MINGW32__
  gui_prodnamelike = gtk_entry_new ();
#else
  gui_prodnamelike = gtk_search_entry_new ();
#endif
  gtk_widget_set_tooltip_text (gui_prodnamelike,
			       "Type some characters and press Enter to filter the product list below. Leave this empty to see all products in the list.");
  g_signal_connect (gui_prodnamelike, "activate", G_CALLBACK (prodnamelike), NULL);
  gtk_grid_attach_next_to (GTK_GRID (grid), gui_prodnamelike, label, GTK_POS_RIGHT, 1, 1);
  gtk_grid_attach_next_to (GTK_GRID (vgrid), grid, NULL, GTK_POS_BOTTOM, 1, 1);

  gtk_grid_attach_next_to (GTK_GRID (vgrid), gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), NULL, GTK_POS_BOTTOM, 1,
			   1);

  gui_prod = -1;
  grid = gtk_grid_new ();
  gui_prod_combo = gtk_combo_box_new ();
  gtk_widget_set_tooltip_text (gui_prod_combo, "Choose a product to edit, and to see its contents.");
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (gui_prod_combo), cell, FALSE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (gui_prod_combo), cell, "text", PRODUCT_NAME);
  g_signal_connect (gui_prod_combo, "changed", G_CALLBACK (product_changed), NULL);
  gtk_grid_attach_next_to (GTK_GRID (grid), gui_prod_combo, NULL, GTK_POS_RIGHT, 1, 1);
  label = gtk_label_new (_(" costs "));
  gtk_grid_attach_next_to (GTK_GRID (grid), label, NULL, GTK_POS_RIGHT, 1, 1);
  gui_price_entry = gtk_entry_new ();
  g_signal_connect (gui_price_entry, "activate", G_CALLBACK (product_price_changed), NULL);
  gtk_widget_set_tooltip_text (gui_price_entry,
			       "Type the cost of the product and press Enter here. There is only an amount, no currency.");
  gtk_grid_attach_next_to (GTK_GRID (grid), gui_price_entry, NULL, GTK_POS_RIGHT, 1, 1);
  label = gtk_label_new (_(", and contains:"));
  gtk_grid_attach_next_to (GTK_GRID (grid), label, NULL, GTK_POS_RIGHT, 1, 1);
  gui_prod_spinner = gtk_spinner_new ();
  gtk_grid_attach_next_to (GTK_GRID (grid), gui_prod_spinner, NULL, GTK_POS_RIGHT, 1, 1);

  button = gtk_button_new_with_label (_("Delete product"));
  gtk_widget_set_tooltip_text (button, "Press this button to delete the currently choosen product.");
  gtk_widget_set_hexpand (button, TRUE);
  gtk_widget_set_halign (button, GTK_ALIGN_END);
  g_signal_connect (button, "clicked", G_CALLBACK (delete_product), NULL);
  gtk_grid_attach_next_to (GTK_GRID (grid), button, NULL, GTK_POS_RIGHT, 1, 1);
  gtk_grid_attach_next_to (GTK_GRID (vgrid), grid, NULL, GTK_POS_BOTTOM, 1, 1);
  gtk_grid_attach_next_to (GTK_GRID (vgrid), gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), NULL, GTK_POS_BOTTOM, 1,
			   1);

  hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  grid = gtk_grid_new ();
  button = gtk_button_new_with_label (_("Add food item"));
  gtk_widget_set_tooltip_text (button,
			       "Press this button to add the current food item (choosen under Food tab) to the current product.");
  gtk_widget_set_halign (button, GTK_ALIGN_START);
  g_signal_connect (button, "clicked", G_CALLBACK (add_food_item), NULL);
  gtk_grid_attach (GTK_GRID (grid), button, 0, 0, 1, 1);
  button = gtk_button_new_with_label (_("Remove food item"));
  gtk_widget_set_tooltip_text (button,
			       "Press this button to remove the food item (choosen below) from the current product.");
  gtk_widget_set_hexpand (button, TRUE);
  gtk_widget_set_halign (button, GTK_ALIGN_START);
  g_signal_connect (button, "clicked", G_CALLBACK (remove_food_item), NULL);
  gtk_grid_attach_next_to (GTK_GRID (grid), button, NULL, GTK_POS_RIGHT, 1, 1);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_size_request (sw, 300, 400);
  gui_ingredients = gtk_tree_view_new ();

  cell = gtk_cell_renderer_text_new ();
  g_value_init (&editable, G_TYPE_BOOLEAN);
  g_value_set_boolean (&editable, TRUE);
  g_object_set_property (G_OBJECT (cell), "editable", &editable);
  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_ingredients),
			       gtk_tree_view_column_new_with_attributes (_("Weight [oz]"), cell, "text", ING_AMOUNT_OZ,
									 NULL));
  g_signal_connect (cell, "edited", G_CALLBACK (ingredient_amount_edited_oz), NULL);
  cell = gtk_cell_renderer_text_new ();
  g_object_set_property (G_OBJECT (cell), "editable", &editable);
  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_ingredients),
			       gtk_tree_view_column_new_with_attributes (_("Weight [g]"), cell, "text", ING_AMOUNT,
									 NULL));
  g_signal_connect (cell, "edited", G_CALLBACK (ingredient_amount_edited_g), NULL);
  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_ingredients),
			       gtk_tree_view_column_new_with_attributes (_("NDBNo"), cell, "text", ING_NDBNO, NULL));
  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_ingredients),
			       gtk_tree_view_column_new_with_attributes (_("Desc"), cell, "text", ING_DESC, NULL));

  gtk_container_add (GTK_CONTAINER (sw), gui_ingredients);
  gtk_grid_attach_next_to (GTK_GRID (grid), sw, NULL, GTK_POS_BOTTOM, 2, 1);
  gtk_widget_set_hexpand (sw, TRUE);
  gtk_widget_set_vexpand (sw, TRUE);
  gtk_widget_set_halign (sw, GTK_ALIGN_FILL);
  gtk_widget_set_valign (sw, GTK_ALIGN_FILL);
  gtk_paned_pack1 (GTK_PANED (hpaned), grid, TRUE, TRUE);
  sw = nutree (&gui_prod_contents);
  gtk_paned_pack2 (GTK_PANED (hpaned), sw, TRUE, TRUE);
  gtk_widget_set_hexpand (sw, TRUE);
  gtk_widget_set_vexpand (sw, TRUE);
  gtk_widget_set_halign (sw, GTK_ALIGN_FILL);
  gtk_widget_set_valign (sw, GTK_ALIGN_FILL);
  gtk_grid_attach_next_to (GTK_GRID (vgrid), hpaned, NULL, GTK_POS_BOTTOM, 1, 1);
  gtk_paned_set_position (GTK_PANED (hpaned), 300);

  return vgrid;
}

GtkWidget *
plantab ()
{
  GtkWidget *vgrid, *grid, *grid1, *grid2, *button, *label, *sw, *hpaned, *frame, *table;
  GtkCellRenderer *cell;
  GValue editable = { 0 };
  GValue digits = { 0 };
  GValue adv = { 0 };
  GtkAdjustment *adj;
  gchar *title;

  vgrid = gtk_grid_new ();
  grid = gtk_grid_new ();
  label = gtk_label_new (_("Plan for "));
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
  adj = gtk_adjustment_new (7, 1, 30, 1, 0, 0);
  gui_plan_days = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  g_signal_connect (gui_plan_days, "value-changed", G_CALLBACK (plan_days_changed), NULL);
  gtk_widget_set_tooltip_text (gui_plan_days, "Set the length (in days) of the diet plan for calculating DRI values.");
  gtk_grid_attach_next_to (GTK_GRID (grid), gui_plan_days, label, GTK_POS_RIGHT, 1, 1);
  label = gtk_label_new (_(" days. "));
  gtk_grid_attach_next_to (GTK_GRID (grid), label, gui_plan_days, GTK_POS_RIGHT, 1, 1);
  gtk_grid_attach (GTK_GRID (vgrid), grid, 0, 0, 1, 1);
  gtk_grid_attach_next_to (GTK_GRID (vgrid), gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), NULL, GTK_POS_BOTTOM, 1,
			   1);

  hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  grid = gtk_grid_new ();
  button = gtk_button_new_with_label (_("Add product"));
  gtk_widget_set_tooltip_text (button,
			       "Press this button to add the current product (choosen under Product tab) to the plan.");
  g_signal_connect (button, "clicked", G_CALLBACK (plan_product), NULL);
  gtk_grid_attach_next_to (GTK_GRID (grid), button, NULL, GTK_POS_RIGHT, 1, 1);
  button = gtk_button_new_with_label (_("Remove product"));
  gtk_widget_set_tooltip_text (button, "Press this button to remove the product (choosen below) from the plan.");
  gtk_widget_set_hexpand (button, TRUE);
  gtk_widget_set_halign (button, GTK_ALIGN_START);
  g_signal_connect (button, "clicked", G_CALLBACK (unplan_product), NULL);
  gtk_grid_attach_next_to (GTK_GRID (grid), button, NULL, GTK_POS_RIGHT, 1, 1);
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_hexpand (sw, TRUE);
  gtk_widget_set_vexpand (sw, TRUE);
  gtk_widget_set_halign (sw, GTK_ALIGN_FILL);
  gtk_widget_set_valign (sw, GTK_ALIGN_FILL);
  gtk_widget_set_size_request (sw, 300, 400);
  gui_plan = gtk_tree_view_new ();

  cell = gtk_cell_renderer_text_new ();
  g_value_init (&editable, G_TYPE_BOOLEAN);
  g_value_set_boolean (&editable, TRUE);
  g_object_set_property (G_OBJECT (cell), "editable", &editable);
  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_plan),
			       gtk_tree_view_column_new_with_attributes (_("Quantity"), cell, "text", PLN_QUANTITY,
									 NULL));
  g_signal_connect (cell, "edited", G_CALLBACK (plan_quantity_edited), NULL);
  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_append_column (GTK_TREE_VIEW (gui_plan),
			       gtk_tree_view_column_new_with_attributes (_("Product"), cell, "text", PLN_PRODUCT_NAME,
									 NULL));
  gtk_container_add (GTK_CONTAINER (sw), gui_plan);
  gtk_grid_attach_next_to (GTK_GRID (grid), sw, NULL, GTK_POS_BOTTOM, 2, 1);
  gtk_paned_pack1 (GTK_PANED (hpaned), grid, TRUE, TRUE);
  sw = nutree (&gui_plan_contents);
  gtk_widget_set_hexpand (sw, TRUE);
  gtk_widget_set_vexpand (sw, TRUE);
  gtk_widget_set_halign (sw, GTK_ALIGN_FILL);
  gtk_widget_set_valign (sw, GTK_ALIGN_FILL);
  gtk_paned_pack2 (GTK_PANED (hpaned), sw, TRUE, TRUE);
  gtk_paned_set_position (GTK_PANED (hpaned), 300);

  gtk_grid_attach_next_to (GTK_GRID (vgrid), hpaned, NULL, GTK_POS_BOTTOM, 1, 1);
  return vgrid;
}

GtkWidget *
eatertab ()
{
  GtkWidget *grid, *vbox, *hbox, *vbox1, *hbox1, *combo, *label, *hpaned, *frame, *table;
  gchar *groups[6] = { N_("Infants"), N_("Children"), N_("Males"), N_("Females"), N_("Pregnancy"), N_("Lactation") };
  gchar *weight_units[2] = { N_("Kg"), N_("lb") };
  gint i;

  grid = gtk_grid_new ();

  label = gtk_label_new (_("Life Stage Group:"));
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
  combo = gtk_combo_box_text_new ();
  for (i = 0; i != 6; i++)
    gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (combo), NULL, gettext (groups[i]));
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 2);
  gtk_grid_attach_next_to (GTK_GRID (grid), combo, label, GTK_POS_RIGHT, 1, 1);
  g_signal_connect (combo, "changed", G_CALLBACK (lsg_changed), NULL);

  label = gtk_label_new (_("Age:"));
  gtk_grid_attach_next_to (GTK_GRID (grid), label, NULL, GTK_POS_BOTTOM, 1, 1);
  gui_age_spin = gtk_spin_button_new_with_range (9.0, 120.0, 1.0);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (gui_age_spin), 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (gui_age_spin), gui_age);
  g_signal_connect (gui_age_spin, "value-changed", G_CALLBACK (age_changed), NULL);
  gtk_grid_attach_next_to (GTK_GRID (grid), gui_age_spin, label, GTK_POS_RIGHT, 1, 1);
  gui_age_unit = gtk_label_new (_("year"));
  gtk_grid_attach_next_to (GTK_GRID (grid), gui_age_unit, gui_age_spin, GTK_POS_RIGHT, 1, 1);

  label = gtk_label_new (_("Weight:"));
  gtk_grid_attach_next_to (GTK_GRID (grid), label, NULL, GTK_POS_BOTTOM, 1, 1);
  gui_weight_spin = gtk_spin_button_new_with_range (0.0, 200.0, 1.0);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (gui_weight_spin), 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (gui_weight_spin), gui_weight);
  g_signal_connect (gui_weight_spin, "value-changed", G_CALLBACK (weight_changed), NULL);
  gtk_grid_attach_next_to (GTK_GRID (grid), gui_weight_spin, label, GTK_POS_RIGHT, 1, 1);
  combo = gtk_combo_box_text_new ();
  for (i = 0; i != 2; i++)
    gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (combo), NULL, gettext (weight_units[i]));
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  g_signal_connect (combo, "changed", G_CALLBACK (weight_unit_changed), NULL);
  gtk_grid_attach_next_to (GTK_GRID (grid), combo, gui_weight_spin, GTK_POS_RIGHT, 1, 1);

  return grid;
}


GtkWidget *
notebook ()
{
  GtkWidget *notebook;

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), foodtab (), gtk_label_new (_("Food")));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), producttab (), gtk_label_new (_("Product")));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), plantab (), gtk_label_new (_("Plan")));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), eatertab (), gtk_label_new (_("Eater")));

  return notebook;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *grid, *box, *menu_item, *menu_bar;
  GError *error = NULL;

  gtk_init (&argc, &argv);
  db_open ();
  init();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), _("Nutrika 1.0"));
  g_signal_connect (window, "delete-event", G_CALLBACK (quit), NULL);
  g_signal_connect (window, "destroy", G_CALLBACK (quit), NULL);
  gtk_window_set_icon (GTK_WINDOW (window), gdk_pixbuf_new_from_file (NUTRIKA_ICON, &error));

  grid = gtk_grid_new ();
  gtk_grid_attach (GTK_GRID (grid), notebook (), 0, 0, 1, 1);
  gtk_container_add (GTK_CONTAINER (window), grid);
  gtk_widget_show_all (window);
  foodgroup_changed (GTK_COMBO_BOX (gui_foodgroup), NULL);
  prodnamelike (NULL, NULL);
  update_plan_contents ();
  gtk_widget_set_size_request (window, 800, 600);
  gtk_main ();

  db_close ();

  return 0;
}
