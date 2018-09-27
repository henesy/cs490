#include <stdio.h>
#include <gtk/gtk.h>

#define MIN_VOL   0
#define MAX_VOL   50
#define MIN_BASS  0
#define MAX_BASS  50
#define MIN_TREB  0
#define MAX_TREB  50
#define MIN_BAL  -50
#define MAX_BAL   50

static gboolean delete( GtkWidget *widget,
                        GtkWidget *event,
                        gpointer   data )
{
    gtk_main_quit();
    return FALSE;
}

enum zones {
  zone_living_room,
  zone_dining_room,
  zone_loft,
  zone_downstairs,
  zone_garage,
  zone_basement,
  num_zones
};

enum inputs {
  input_cdplayer,
  input_chromecast,
  input_tuner,
  num_inputs
};

enum adjustment {
  adj_volume,
  adj_bass,
  adj_treble,
  adj_balance,
  num_adjustments
};

int main( int argc,
          char *argv[] )
{
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *table;
    GtkWidget *notebook;
    GtkWidget *frame[num_zones];
    GtkWidget *label;
    GtkWidget *combo;
    GtkWidget *scale;
    GtkWidget *links;
    GtkObject *adjustment[num_zones][num_adjustments];
    GtkWidget *zone_link[num_zones][num_zones];
    GList *inputs;
    

    int i;
    char *rooms[6] = {
      "Family Room",
      "Dining Room",
      "Loft",
      "Conservatory",
      "Garage",
      "Basement"
    };

    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    /*    gtk_window_fullscreen(GTK_WINDOW(window));*/
    gtk_widget_set_size_request(window, 800, 480);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    
    g_signal_connect(window, "delete-event",
	              G_CALLBACK(delete), NULL);
    
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    
    /* Create a new notebook, place the position of the tabs */
    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    gtk_widget_show(notebook);
    
    inputs = NULL;
    inputs = g_list_append(inputs, "CD Player");
    inputs = g_list_append(inputs, "Chromecast");
    inputs = g_list_append(inputs, "Tuner");

    /* Let's append a bunch of pages to the notebook */
    for (i = 0; i < 6; i++) {
	frame[i] = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame[i]), 10);
	gtk_widget_show(frame[i]);
	label = gtk_label_new(rooms[i]);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame[i], label);

        table = gtk_table_new(15, 10, TRUE);
        gtk_container_add(GTK_CONTAINER(frame[i]), table);
        gtk_widget_show(table);

        button = gtk_toggle_button_new_with_label("Power");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
        gtk_table_attach_defaults(GTK_TABLE(table), button, 0, 2, 0, 2);
        gtk_widget_show(button);

        button = gtk_toggle_button_new_with_label("Mute");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
        gtk_table_attach_defaults(GTK_TABLE(table), button, 6, 7, 0, 2);
        gtk_widget_show(button);

        combo = gtk_combo_box_text_new();
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo),
                                       "CD Player");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo),
                                       "Chromecast");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo),
                                       "Tuner");
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

        /*
        combo = gtk_combo_new();
        gtk_combo_set_popdown_strings(GTK_COMBO(combo), inputs);
        */
        gtk_table_attach_defaults(GTK_TABLE(table), combo, 3, 5, 1, 2);
        gtk_widget_show(combo);
        label = gtk_label_new("Input Channel");
        gtk_table_attach_defaults(GTK_TABLE(table), label, 3, 5, 0, 1);
        gtk_widget_show(label);

        adjustment[i][0] = gtk_adjustment_new(10, MIN_VOL,
                                              MAX_VOL + 1, 1, 1, 1);
        label = gtk_label_new("Volume");
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 3, 4);
        gtk_widget_show(label);
        scale = gtk_hscale_new(GTK_ADJUSTMENT(adjustment[i][0]));
        gtk_range_set_update_policy(GTK_RANGE(scale),
                                    GTK_UPDATE_DISCONTINUOUS);
        gtk_scale_set_digits(GTK_SCALE(scale), 0);
        gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_LEFT);
        gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);
        gtk_table_attach_defaults(GTK_TABLE(table), scale, 1, 7, 3, 4);
        gtk_range_set_update_policy(GTK_RANGE(scale), GTK_UPDATE_DISCONTINUOUS);
        gtk_widget_show(scale);

        adjustment[i][1] = gtk_adjustment_new(10, MIN_BASS,
                                              MAX_BASS + 1, 1, 1, 1);
        label = gtk_label_new("Bass");
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 6, 7);
        gtk_widget_show(label);
        scale = gtk_hscale_new(GTK_ADJUSTMENT(adjustment[i][1]));
        gtk_range_set_update_policy(GTK_RANGE(scale),
                                    GTK_UPDATE_DISCONTINUOUS);
        gtk_scale_set_digits(GTK_SCALE(scale), 0);
        gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_LEFT);
        gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);

        gtk_table_attach_defaults(GTK_TABLE(table), scale, 1, 7, 6, 7);
        gtk_range_set_update_policy(GTK_RANGE(scale), GTK_UPDATE_DISCONTINUOUS);
        gtk_widget_show(scale);

        adjustment[i][2] = gtk_adjustment_new(10, MIN_TREB,
                                              MAX_TREB + 1, 1, 1, 1);
        label = gtk_label_new("Treble");
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 9, 10);
        gtk_widget_show(label);
        scale = gtk_hscale_new(GTK_ADJUSTMENT(adjustment[i][2]));
        gtk_range_set_update_policy(GTK_RANGE(scale),
                                    GTK_UPDATE_DISCONTINUOUS);
        gtk_scale_set_digits(GTK_SCALE(scale), 0);
        gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_LEFT);
        gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);

        gtk_table_attach_defaults(GTK_TABLE(table), scale, 1, 7, 9, 10);
        gtk_range_set_update_policy(GTK_RANGE(scale), GTK_UPDATE_DISCONTINUOUS);
        gtk_widget_show(scale);

        adjustment[i][3] = gtk_adjustment_new(0, MIN_BAL,
                                              MAX_BAL + 1, 1, 1, 1);
        label = gtk_label_new("Balance");
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 12, 13);
        gtk_widget_show(label);
        scale = gtk_hscale_new(GTK_ADJUSTMENT(adjustment[i][3]));
        gtk_range_set_update_policy(GTK_RANGE(scale),
                                    GTK_UPDATE_DISCONTINUOUS);
        gtk_scale_set_digits(GTK_SCALE(scale), 0);
        gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_LEFT);
        gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);

        gtk_table_attach_defaults(GTK_TABLE(table), scale, 1, 7, 12, 13);
        gtk_range_set_update_policy(GTK_RANGE(scale), GTK_UPDATE_DISCONTINUOUS);
        gtk_widget_show(scale);

        links = gtk_table_new(1, 7, FALSE);
        gtk_table_attach_defaults(GTK_TABLE(table), links, 8, 9, 0, 15);
        gtk_widget_show(links);

        label = gtk_label_new("Link with:");
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_table_attach_defaults(GTK_TABLE(links), label, 0, 1, 0, 1);
        gtk_widget_show(label);

        button = gtk_check_button_new_with_label("Family Room");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
        gtk_table_attach_defaults(GTK_TABLE(links), button, 0, 1, 1, 2);
        gtk_widget_show(button);

        button = gtk_check_button_new_with_label("Dining Room");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
        gtk_table_attach_defaults(GTK_TABLE(links), button, 0, 1, 2, 3);
        gtk_widget_show(button);

        button = gtk_check_button_new_with_label("Loft");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
        gtk_table_attach_defaults(GTK_TABLE(links), button, 0, 1, 3, 4);
        gtk_widget_show(button);

        button = gtk_check_button_new_with_label("Conservatory");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
        gtk_table_attach_defaults(GTK_TABLE(links), button, 0, 1, 4, 5);
        gtk_widget_show(button);

        button = gtk_check_button_new_with_label("Garage");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
        gtk_table_attach_defaults(GTK_TABLE(links), button, 0, 1, 5, 6);
        gtk_widget_show(button);

        button = gtk_check_button_new_with_label("Basement");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
        gtk_table_attach_defaults(GTK_TABLE(links), button, 0, 1, 6, 7);
        gtk_widget_show(button);

    }
      
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);
    
    gtk_container_add(GTK_CONTAINER(window), notebook);
    gtk_widget_show(window);
    
    /*
    GTK_ADJUSTMENT(adjustment[0][0])->value = 20;
    gtk_signal_emit_by_name(GTK_OBJECT(adjustment[0][0]), "changed");
    */

    /*
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 2);
    */
 
    gtk_main();
    
    return 0;
}
