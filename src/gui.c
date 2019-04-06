
/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007 Tomasz Maka <pasp@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "gui.h"
#include "about.h"
#include "i18n.h"
#include "utils.h"
#include "utils_gui.h"
#include "utils_date.h"
#include "calendar.h"
#include "calendar_notes.h"
#include "calendar_fullyear.h"
#include "calendar_widget.h"
#include "calendar_jumpto.h"
#include "calendar_calc.h"
#ifdef HAVE_LIBICAL
#include "calendar_ical.h"
#endif  /* HAVE_LIBICAL */
#include "calendar_utils.h"
#include "notes.h"
#include "notes_items.h"
#include "tasks.h"
#include "tasks_items.h"
#include "tasks_utils.h"
#include "notes_items.h"
#include "contacts.h"
#include "contacts_items.h"
#include "options_prefs.h"
#include "check_events.h"
#include "stock_icons.h"
#include "preferences_gui.h"
#include "calendar_preferences_gui.h"
#include "config.h"
#ifdef HAVE_GTKSPELL
#include <gtkspell/gtkspell.h>
#endif /* HAVE_GTKSPELL */

#include "gui_icon.h"

struct  osmo_prefs              config;

/*------------------------------------------------------------------------------*/

void
gui_save_data_and_run_command (gchar *command, GUI *appGUI) {
    gui_save_all_data (appGUI);
    utl_run_command (command, FALSE);
}

/*------------------------------------------------------------------------------*/

void
gui_save_all_data (GUI *appGUI) {

    cal_write_notes (appGUI);
#ifdef TASKS_ENABLED
	store_task_columns_info (appGUI);
    write_tasks_entries (appGUI);
#endif  /* TASKS_ENABLED */
#ifdef HAVE_LIBICAL
    write_ical_entries (appGUI);
    ics_calendar_refresh (appGUI);
#endif  /* HAVE_LIBICAL */
#ifdef CONTACTS_ENABLED
	store_contact_columns_info (appGUI);
    write_contacts_entries (appGUI);
#endif  /* CONTACTS_ENABLED */
#ifdef NOTES_ENABLED
	store_note_columns_info (appGUI);
    write_notes_entries (appGUI);
#endif  /* NOTES_ENABLED */
    prefs_write_config (appGUI);

}

/*------------------------------------------------------------------------------*/

void
gui_toggle_window_visibility (GUI *appGUI) {

    if (appGUI->no_tray == FALSE) {

        appGUI->window_visible = !appGUI->window_visible;

        if (appGUI->window_visible == TRUE) {
            gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_NORMAL);
            gtk_window_set_default_size (GTK_WINDOW(appGUI->main_window), 
										 config.window_size_x, config.window_size_y);
            gtk_window_move (GTK_WINDOW (appGUI->main_window), config.window_x, config.window_y);
            gtk_widget_show (appGUI->main_window);
        } else {
            if (appGUI->calendar_only == FALSE) {
                gui_save_all_data (appGUI);
				gtk_window_get_size (GTK_WINDOW(appGUI->main_window),
									&config.window_size_x, &config.window_size_y);
				gdk_window_get_root_origin (GDK_WINDOW(appGUI->main_window->window),
									&config.window_x, &config.window_y);
            }
            gtk_widget_hide (appGUI->main_window);
        }
    }
}

/*------------------------------------------------------------------------------*/

void
gui_quit_osmo (GUI *appGUI) {

#ifdef NOTES_ENABLED
    if (appGUI->nte->editor_active == TRUE) {
        g_signal_emit_by_name (G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/close")), "clicked");
    }
#endif  /* NOTES_ENABLED */

    if (appGUI->osmo_trayicon != NULL) {
        gtk_status_icon_set_visible (appGUI->osmo_trayicon, FALSE);
    }

    if (appGUI->calendar_only == FALSE) {
        if (appGUI->current_tab == PAGE_CALENDAR 
#ifdef TASKS_ENABLED
            || appGUI->current_tab == PAGE_TASKS 
#endif  /* TASKS ENABLED */
#if defined(NOTES_ENABLED) && defined(CONTACTS_ENABLED)
            || appGUI->current_tab == PAGE_CONTACTS || appGUI->current_tab == PAGE_NOTES) {
#elif CONTACTS_ENABLED
            || appGUI->current_tab == PAGE_CONTACTS) {
#else
            ) {
#endif  /* NOTES_ENABLED && CONTACTS_ENABLED */
            config.latest_tab = appGUI->current_tab;
        }

#ifdef CONTACTS_ENABLED
        config.find_mode = gtk_combo_box_get_active (GTK_COMBO_BOX(appGUI->cnt->contacts_find_combobox));
		if (config.gui_layout) {
            config.contacts_pane_pos = gtk_paned_get_position(GTK_PANED(appGUI->cnt->contacts_paned));
		}
#endif  /* CONTACTS_ENABLED */

#ifdef TASKS_ENABLED
        config.current_category_in_tasks = gtk_combo_box_get_active (GTK_COMBO_BOX(appGUI->tsk->cf_combobox));
		if (config.gui_layout) {
	        config.tasks_pane_pos = gtk_paned_get_position(GTK_PANED(appGUI->tsk->tasks_paned));
		}
#endif  /* TASKS_ENABLED */

#ifdef NOTES_ENABLED
        config.current_category_in_notes = gtk_combo_box_get_active (GTK_COMBO_BOX(appGUI->nte->cf_combobox));
#endif  /* NOTES_ENABLED */

        gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(appGUI->cal->notes_button), FALSE);

        if (appGUI->window_visible == TRUE) {
            gtk_window_get_size (GTK_WINDOW(appGUI->main_window),
                                &config.window_size_x, &config.window_size_y);
        }
    } 

    if (appGUI->window_visible == TRUE) {
        gdk_window_get_root_origin (GDK_WINDOW(appGUI->main_window->window),
                            &config.window_x, &config.window_y);
    }

    pango_font_description_free(appGUI->cal->fd_day_name_font);
    pango_font_description_free(appGUI->cal->fd_cal_font);
    pango_font_description_free(appGUI->cal->fd_notes_font);

    if (appGUI->calendar_only == FALSE) {
        config.lastrun_date = utl_date_get_current_julian ();
        config.lastrun_time = utl_time_get_current_seconds ();
        gui_save_all_data (appGUI);
#ifdef HAVE_LIBNOTIFY
#ifdef TASKS_ENABLED
        free_notifications_list (appGUI);
#endif  /* TASKS_ENABLED */
#endif  /* HAVE_LIBNOTIFY */
        cal_free_notes_list (appGUI);
    }

    utl_gui_url_remove_links (&appGUI->about_links_list, &appGUI->about_link_index);
    gtk_main_quit ();
}

/*------------------------------------------------------------------------------*/

void
gui_window_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    if (config.enable_systray == TRUE && appGUI->no_tray == FALSE && appGUI->calendar_only == FALSE) {
        gui_toggle_window_visibility (appGUI);
    } else {
        gui_quit_osmo (appGUI);
    }
}

/*------------------------------------------------------------------------------*/

void
gui_activate_find_fields (GUI *appGUI) {

#ifdef NOTES_ENABLED
    if (appGUI->current_tab == PAGE_NOTES) {
		gtk_widget_grab_focus (GTK_WIDGET(appGUI->nte->notes_find_entry));
	}
#endif  /* NOTES_ENABLED */

#ifdef TASKS_ENABLED
    if (appGUI->current_tab == PAGE_TASKS) {
		gtk_widget_grab_focus (GTK_WIDGET(appGUI->tsk->tasks_find_entry));
    }
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
    if (appGUI->current_tab == PAGE_CONTACTS) {
        gtk_widget_grab_focus (GTK_WIDGET(appGUI->cnt->contacts_find_entry));
    }
#endif  /* CONTACTS_ENABLED */

}

/*------------------------------------------------------------------------------*/

gint
get_first_active_page (GUI *appGUI) {

    if (!config.hide_calendar) 
        return PAGE_CALENDAR;
#ifdef TASKS_ENABLED
    if (!config.hide_tasks) 
        return PAGE_TASKS;
#endif  /* TASKS_ENABLED */
#ifdef CONTACTS_ENABLED
    if (!config.hide_contacts) 
        return PAGE_CONTACTS;
#endif  /* CONTACTS_ENABLED */
#ifdef NOTES_ENABLED
    if (!config.hide_notes) 
        return PAGE_NOTES;
#endif  /* NOTES_ENABLED */

    return PAGE_CALENDAR;
}

/*------------------------------------------------------------------------------*/

void
set_visible_page (gint page, gboolean dir, GUI *appGUI) {

gboolean flag;
gint n = 1;

    if (dir == FALSE) n = -1;

    page += n;
    flag = TRUE;

    while (flag) {

        flag = FALSE;

        switch (page) {
            case PAGE_CALENDAR:
                if (config.hide_calendar) {
                    page += n;
                    flag = TRUE;
                }
                break;
#ifdef TASKS_ENABLED
            case PAGE_TASKS:
                if (config.hide_tasks) {
                    page += n;
                    flag = TRUE;
                }
                break;
#endif  /* TASKS_ENABLED */
#ifdef CONTACTS_ENABLED
            case PAGE_CONTACTS:
                if (config.hide_contacts) {
                    page += n;
                    flag = TRUE;
                }
                break;
#endif  /* CONTACTS_ENABLED */
#ifdef NOTES_ENABLED
            case PAGE_NOTES:
                if (config.hide_notes) {
                    page += n;
                    flag = TRUE;
                }
                break;
#endif  /* NOTES_ENABLED */
        }
    }

    if (dir == FALSE) {
        appGUI->current_tab = (page < 0) ? appGUI->number_of_tabs-1:page;
    } else {
        appGUI->current_tab = (page == appGUI->number_of_tabs) ? get_first_active_page(appGUI):page;
    }
    gtk_notebook_set_current_page (GTK_NOTEBOOK (appGUI->notebook), appGUI->current_tab);

	gui_activate_find_fields (appGUI);

}

/*------------------------------------------------------------------------------*/

static gint
get_visible_tabs (GUI *appGUI)
{
	gint i = 0;

	if (!config.hide_calendar) i++;
#ifdef TASKS_ENABLED
	if (!config.hide_tasks) i++;
#endif  /* TASKS_ENABLED */
#ifdef CONTACTS_ENABLED
	if (!config.hide_contacts) i++;
#endif  /* CONTACTS_ENABLED */
#ifdef NOTES_ENABLED
	if (!config.hide_notes) i++;
#endif  /* NOTES_ENABLED */

	return i;
}

/*------------------------------------------------------------------------------*/

static void
select_tab (gint tab, GUI *appGUI)
{
	gint i, n = 0;

	if (tab >= get_visible_tabs (appGUI)) return;

	for (i = PAGE_CALENDAR; i < NUMBER_OF_TABS; i++) {

		if (i == PAGE_CALENDAR && !config.hide_calendar) n++;
#ifdef TASKS_ENABLED
		if (i == PAGE_TASKS && !config.hide_tasks) n++;
#endif  /* TASKS_ENABLED */
#ifdef CONTACTS_ENABLED
		if (i == PAGE_CONTACTS && !config.hide_contacts) n++;
#endif  /* CONTACTS_ENABLED */
#ifdef NOTES_ENABLED
		if (i == PAGE_NOTES && !config.hide_notes) n++;
#endif  /* NOTES_ENABLED */

		if (n == tab + 1) {
			gtk_notebook_set_current_page (GTK_NOTEBOOK (appGUI->notebook), i);
			break;
		}
	}

	gui_activate_find_fields (appGUI);
}

/*------------------------------------------------------------------------------*/

void
key_counter_add (gint value, GUI *appGUI) {

GtkWidget *dialog = NULL;
gchar tmpbuff[BUFFER_SIZE];

    appGUI->key_counter += value;

    if (appGUI->key_counter == 57) {
	    g_snprintf (tmpbuff, BUFFER_SIZE, "<span size='xx-large'><b>%d times!</b></span>", config.run_counter);

        dialog = gtk_dialog_new_with_buttons ("Counter", GTK_WINDOW(appGUI->main_window),
                                              GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                              OSMO_STOCK_BUTTON_CLOSE, GTK_RESPONSE_NO, NULL);
        utl_gui_fill_iconlabel (dialog, OSMO_STOCK_INFO_HELP, tmpbuff);
        gtk_widget_show (dialog);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else if (appGUI->key_counter == 41) {
		appGUI->cal->datecal_bio = TRUE;
	}
}

/*------------------------------------------------------------------------------*/

gint 
key_press_cb (GtkWidget *widget, GdkEventKey *event, GUI *appGUI)
{
	gint page;

    page = gtk_notebook_get_current_page (GTK_NOTEBOOK(appGUI->notebook));

    /************************************************************************/
    /*** CALENDAR PAGE                                                    ***/
    /************************************************************************/

    if(page == PAGE_CALENDAR) {

        if (!config.day_notes_visible) {

            switch (event->keyval) {
                case GDK_Left:
                    calendar_btn_prev_day(appGUI);
                    return TRUE;
                case GDK_Right:
                    calendar_btn_next_day(appGUI);
                    return TRUE;
                case GDK_Up:
                    if (event->state & GDK_CONTROL_MASK) {  /* CTRL + Up */
                        utl_gui_sw_vscrollbar_move_position (appGUI->cal->day_info_scrolledwindow, SW_MOVE_UP);
                        return TRUE;
                    } else {
                        calendar_btn_prev_week(appGUI);
                        return TRUE;
                    }
                    return FALSE;
                case GDK_Down:
                    if (event->state & GDK_CONTROL_MASK) {  /* CTRL + Up */
                        utl_gui_sw_vscrollbar_move_position (appGUI->cal->day_info_scrolledwindow, SW_MOVE_DOWN);
                        return TRUE;
                    } else {
                        calendar_btn_next_week(appGUI);
                        return TRUE;
                    }
                    return FALSE;
                case GDK_Home:
                    calendar_btn_prev_year(appGUI);
                    return TRUE;
                case GDK_End:
                    calendar_btn_next_year(appGUI);
                    return TRUE;
                case GDK_Return:
                    if (appGUI->calendar_only == FALSE) {
                        gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(appGUI->cal->notes_button), 
                                                           !gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(appGUI->cal->notes_button)));
                    }
                    return TRUE;
                case GDK_space:
                    if (event->state & GDK_CONTROL_MASK) {  /* CTRL + Space */
                        config.enable_day_mark = !config.enable_day_mark;
                        cal_refresh_marks (appGUI);
                        cal_set_day_info (appGUI);
                        update_aux_calendars (appGUI);
                        return TRUE;
                    } else {
                        calendar_set_today (appGUI);
                        return TRUE;
                    }
                case GDK_Delete:
                    if (appGUI->calendar_only == FALSE) {
                        calendar_clear_text_cb (NULL, appGUI);
                    }
                    return TRUE;
                case GDK_g:
                    calendar_create_jumpto_window (appGUI);
                    return TRUE;
                case GDK_f:
                    calendar_create_fullyear_window (appGUI);
                    return TRUE;
                case GDK_c:
                    calendar_create_color_selector_window (TRUE, appGUI);
                    key_counter_add (13, appGUI);
                    return TRUE;
                case GDK_d:
                    calendar_create_calc_window (appGUI);
                    return TRUE;
                case GDK_a:
					if (!config.gui_layout) {
						gtk_expander_set_expanded (GTK_EXPANDER (appGUI->cal->aux_cal_expander),
												   !gtk_expander_get_expanded (GTK_EXPANDER (appGUI->cal->aux_cal_expander)));
					}
                    key_counter_add (5, appGUI);
                    return TRUE;
                case GDK_b:
                    if (appGUI->calendar_only == FALSE) {
                        cal_notes_browser (appGUI);
                    }
                    key_counter_add (11, appGUI);
                    return TRUE;
                case GDK_i:
                    key_counter_add (9, appGUI);
                    return TRUE;
                case GDK_o:
                    key_counter_add (21, appGUI);
                    return TRUE;
            }

        }

        switch (event->keyval) {

            case GDK_Escape:
                if (appGUI->calendar_only == FALSE) {
                    if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(appGUI->cal->notes_button)) == FALSE) {
                        if (config.enable_systray == TRUE && appGUI->no_tray == FALSE && appGUI->calendar_only == FALSE) {
                            gui_toggle_window_visibility (appGUI);
						}
					} else {
	                    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(appGUI->cal->notes_button), FALSE);
					}
                }
                return TRUE;
            case GDK_Left:
                if (event->state & GDK_MOD1_MASK) {  /* ALT + Left */
                    calendar_btn_prev_day(appGUI);
                    return TRUE;
                }
                return FALSE;
            case GDK_Right:
                if (event->state & GDK_MOD1_MASK) {  /* ALT + Right */
                    calendar_btn_next_day(appGUI);
                    return TRUE;
                }
                return FALSE;
            case GDK_Up:
                if (event->state & GDK_MOD1_MASK) {  /* ALT + Up */
                    calendar_btn_prev_week(appGUI);
                    return TRUE;
                }
                return FALSE;
            case GDK_Down:
                if (event->state & GDK_MOD1_MASK) {  /* ALT + Down */
                    calendar_btn_next_week(appGUI);
                    return TRUE;
                }
                return FALSE;
			case GDK_b:
				if (event->state & GDK_CONTROL_MASK) {  /* CTRL + b */
					g_signal_emit_by_name(G_OBJECT(appGUI->cal->ta_bold_button), "clicked");
					return TRUE;
				}
				return FALSE;

			case GDK_i:
				if (event->state & GDK_CONTROL_MASK) {  /* CTRL + i */
					g_signal_emit_by_name(G_OBJECT(appGUI->cal->ta_italic_button), "clicked");
					return TRUE;
				}
				return FALSE;

			case GDK_m:
				if (event->state & GDK_CONTROL_MASK) {  /* CTRL + m */
					g_signal_emit_by_name(G_OBJECT(appGUI->cal->ta_highlight_button), "clicked");
					return TRUE;
				}
				return FALSE;

			case GDK_u:
				if (event->state & GDK_CONTROL_MASK) {  /* CTRL + u */
					g_signal_emit_by_name(G_OBJECT(appGUI->cal->ta_underline_button), "clicked");
					return TRUE;
				}
				return FALSE;

			case GDK_t:
				if (event->state & GDK_CONTROL_MASK) {  /* CTRL + t */
					g_signal_emit_by_name(G_OBJECT(appGUI->cal->ta_strikethrough_button), "clicked");
					return TRUE;
				}
				return FALSE;
        }

    }

#ifdef TASKS_ENABLED

    /************************************************************************/
    /*** TASKS PAGE                                                        ***/
    /************************************************************************/

    if(page == PAGE_TASKS) {

            switch (event->keyval) {

                case GDK_Escape:
                    if(gtk_widget_is_focus(appGUI->tsk->tasks_find_entry) == FALSE) {
						if(appGUI->tsk->tasks_panel_status == TRUE) {
							show_tasks_desc_panel(FALSE, appGUI);
						} else {
							if (config.enable_systray == TRUE && appGUI->no_tray == FALSE && appGUI->calendar_only == FALSE) {
								gui_toggle_window_visibility (appGUI);
							}
						}
					} else {
                        if (strlen(gtk_entry_get_text(GTK_ENTRY(appGUI->tsk->tasks_find_entry)))) {
                            gtk_entry_set_text(GTK_ENTRY(appGUI->tsk->tasks_find_entry), "");
                        } else {
							if (config.enable_systray == TRUE && appGUI->no_tray == FALSE && appGUI->calendar_only == FALSE) {
								gui_toggle_window_visibility (appGUI);
							}
						}
					}
                    return TRUE;
                case GDK_Return:
                    if(gtk_widget_is_focus(appGUI->tsk->tasks_find_entry) == FALSE) {
						if (event->state & GDK_CONTROL_MASK) {  /* CTRL + Enter */
							if (gtk_tree_selection_get_selected (appGUI->tsk->tasks_list_selection, NULL, NULL)) {
								tasks_add_edit_dialog_show (TRUE, 0, utl_time_get_current_seconds (), appGUI);
								return TRUE;
							}
						}
						if(appGUI->tsk->tasks_panel_status == FALSE) {
							show_tasks_desc_panel(TRUE, appGUI);
						}
						return TRUE;
					} else {
                        gtk_widget_grab_focus(GTK_WIDGET(appGUI->tsk->tasks_list));
						return TRUE;
					}
                case GDK_space:     /* don't use space key for marking task as done */
                    if(gtk_widget_is_focus(appGUI->tsk->tasks_find_entry) == FALSE) {
                        return TRUE;
					} else {
						return FALSE;
					}
                case GDK_h:
                    if(gtk_widget_is_focus(appGUI->tsk->tasks_find_entry) == FALSE) {
						if (event->state & GDK_CONTROL_MASK) {  /* CTRL + h */
							config.hide_completed = !config.hide_completed;
							gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(appGUI->tsk->tasks_filter));
							update_tasks_number (appGUI);
						}
						return TRUE;
					}
					return FALSE;
                case GDK_Delete:
                    if(gtk_widget_is_focus(appGUI->tsk->tasks_find_entry) == FALSE) {
                        tasks_remove_dialog_show(appGUI->tsk->tasks_list, appGUI->tsk->tasks_list_store, appGUI);
	                    return TRUE;
					}
	                return FALSE;
                case GDK_Insert:
                    if(gtk_widget_is_focus(appGUI->tsk->tasks_find_entry) == FALSE) {
                        tasks_add_edit_dialog_show (FALSE, 0, utl_time_get_current_seconds (), appGUI);
                        return TRUE;
					}
	                return FALSE;
                case GDK_Left:
                    if(gtk_widget_is_focus(appGUI->tsk->tasks_find_entry) == FALSE) {
						if (appGUI->tsk->filter_index > 0) {
							appGUI->tsk->filter_index--;
							gtk_combo_box_set_active(GTK_COMBO_BOX(appGUI->tsk->cf_combobox), appGUI->tsk->filter_index);
						}
						return TRUE;
					}
	                return FALSE;
                case GDK_Right:
                    if(gtk_widget_is_focus(appGUI->tsk->tasks_find_entry) == FALSE) {
						if (appGUI->tsk->filter_index < utl_gui_get_combobox_items(GTK_COMBO_BOX(appGUI->tsk->cf_combobox))-1) {
							appGUI->tsk->filter_index++;
							gtk_combo_box_set_active(GTK_COMBO_BOX(appGUI->tsk->cf_combobox), appGUI->tsk->filter_index);
						}
						return TRUE;
					}
	                return FALSE;
                case GDK_y:
                    key_counter_add (26, appGUI);
                    return TRUE;
                case GDK_l:
                    if (event->state & GDK_CONTROL_MASK) {  /* CTRL + l */
                        gtk_widget_grab_focus(GTK_WIDGET(appGUI->tsk->tasks_find_entry));
                        return TRUE;
                    }
                    return FALSE;
            }
    }

#endif /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED

    /************************************************************************/
    /*** CONTACTS PAGE                                                    ***/
    /************************************************************************/

    if(page == PAGE_CONTACTS) {

            switch (event->keyval) {

                case GDK_Escape:
                    if (gtk_widget_is_focus(appGUI->cnt->contacts_find_entry) == FALSE) {
                        if(appGUI->cnt->contacts_panel_status == TRUE) {
                            show_contacts_desc_panel(FALSE, appGUI);
                        } else {
							if (config.enable_systray == TRUE && appGUI->no_tray == FALSE && appGUI->calendar_only == FALSE) {
								gui_toggle_window_visibility (appGUI);
							}
						}
                    } else {
                        if (strlen(gtk_entry_get_text(GTK_ENTRY(appGUI->cnt->contacts_find_entry)))) {
                            gtk_entry_set_text(GTK_ENTRY(appGUI->cnt->contacts_find_entry), "");
                        } else {
							if (config.enable_systray == TRUE && appGUI->no_tray == FALSE && appGUI->calendar_only == FALSE) {
								gui_toggle_window_visibility (appGUI);
							}
						}
                    }
                    return TRUE;
                case GDK_Return:
                    if(gtk_widget_is_focus(appGUI->cnt->contacts_find_entry) == FALSE) {
						if (event->state & GDK_CONTROL_MASK) {  /* CTRL + Enter */
							if (gtk_tree_selection_get_selected (appGUI->cnt->contacts_list_selection, NULL, NULL)) {
								contacts_add_edit_dialog_show (TRUE, appGUI);
								return TRUE;
							}
						}
						if(appGUI->cnt->contacts_panel_status == FALSE) {
							show_contacts_desc_panel(TRUE, appGUI);
						} else if (gtk_widget_is_focus(appGUI->cnt->contacts_find_entry) == FALSE) {
							utl_gui_sw_vscrollbar_move_position (appGUI->cnt->contacts_panel_scrolledwindow, SW_MOVE_DOWN);
							return TRUE;
						}
						return TRUE;
					} else {
                        gtk_widget_grab_focus(GTK_WIDGET(appGUI->cnt->contacts_list));
						return TRUE;
					}
                    return FALSE;

                case GDK_BackSpace:
                    if(appGUI->cnt->contacts_panel_status == TRUE && gtk_widget_is_focus(appGUI->cnt->contacts_find_entry) == FALSE) {
                        utl_gui_sw_vscrollbar_move_position (appGUI->cnt->contacts_panel_scrolledwindow, SW_MOVE_UP);
                        return TRUE;
                    }
                    return FALSE;
                case GDK_Delete:
                    if(gtk_widget_is_focus(appGUI->cnt->contacts_find_entry) == FALSE) {
                        contacts_remove_dialog_show(appGUI->cnt->contacts_list, appGUI->cnt->contacts_list_store, appGUI);
                        return TRUE;
                    }
                    return FALSE;
                case GDK_Insert:
                    if(gtk_widget_is_focus(appGUI->cnt->contacts_find_entry) == FALSE) {
                        contacts_add_edit_dialog_show (FALSE, appGUI);
                        return TRUE;
                    }
                    return FALSE;
                case GDK_Down:
                    if (event->state & GDK_CONTROL_MASK) {  /* CTRL + Down */
                        if (config.find_mode < CONTACTS_FF_ALL_FIELDS) {
                            config.find_mode++;
                        }
                        gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->cnt->contacts_find_combobox), config.find_mode);
                        return TRUE;
                    }
                    return FALSE;
                case GDK_Up:
                    if (event->state & GDK_CONTROL_MASK) {  /* CTRL + Up */
                        if (config.find_mode > CONTACTS_FF_FIRST_NAME) {
                            config.find_mode--;
                        }
                        gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->cnt->contacts_find_combobox), config.find_mode);
                        return TRUE;
                    }
                    return FALSE;
                case GDK_l:
                    if (event->state & GDK_CONTROL_MASK) {  /* CTRL + l */
                        gtk_widget_grab_focus(GTK_WIDGET(appGUI->cnt->contacts_find_entry));
                        return TRUE;
                    }
                    return FALSE;
            }
    }

#endif /* CONTACTS_ENABLED */

    /************************************************************************/
    /*** NOTES PAGE                                                       ***/
    /************************************************************************/

#ifdef NOTES_ENABLED
    if(page == PAGE_NOTES) {
        if (appGUI->nte->editor_active == FALSE) {

            /* SELECTOR */
            switch (event->keyval) {

                case GDK_Return:
                    if(gtk_widget_is_focus(appGUI->nte->notes_find_entry) == FALSE) {
						if (gtk_tree_selection_get_selected (appGUI->nte->notes_list_selection, NULL, NULL)) {
							if (event->state & GDK_CONTROL_MASK) {  /* CTRL + Enter */
								notes_edit_dialog_show(appGUI->nte->notes_list, appGUI->nte->notes_filter, appGUI);
							} else {
								notes_enter_password (appGUI);
							}
							return TRUE;
						}
	                    return FALSE;
					} else {
                        gtk_widget_grab_focus(GTK_WIDGET(appGUI->nte->notes_list));
						return TRUE;
					}
                case GDK_Delete:
                    if(gtk_widget_is_focus(appGUI->nte->notes_find_entry) == FALSE) {
                        notes_remove_dialog_show (appGUI->nte->notes_list, appGUI->nte->notes_list_store, appGUI);
                        return TRUE;
					}
                    return FALSE;
                case GDK_Insert:
                    if(gtk_widget_is_focus(appGUI->nte->notes_find_entry) == FALSE) {
                        notes_add_entry (appGUI);
                        return TRUE;
					}
                    return FALSE;
                case GDK_Left:
                    if(gtk_widget_is_focus(appGUI->nte->notes_find_entry) == FALSE) {
						if (appGUI->nte->filter_index > 0) {
							appGUI->nte->filter_index--;
							gtk_combo_box_set_active(GTK_COMBO_BOX(appGUI->nte->cf_combobox), appGUI->nte->filter_index);
						}
                        return TRUE;
					}
					return FALSE;
                case GDK_Right:
                    if(gtk_widget_is_focus(appGUI->nte->notes_find_entry) == FALSE) {
						if (appGUI->nte->filter_index < utl_gui_get_combobox_items(GTK_COMBO_BOX(appGUI->nte->cf_combobox))-1) {
							appGUI->nte->filter_index++;
							gtk_combo_box_set_active(GTK_COMBO_BOX(appGUI->nte->cf_combobox), appGUI->nte->filter_index);
						}
                        return TRUE;
					}
					return FALSE;
                case GDK_Escape:
                    if(gtk_widget_is_focus(appGUI->nte->notes_find_entry) == FALSE) {
						if (config.enable_systray == TRUE && appGUI->no_tray == FALSE && appGUI->calendar_only == FALSE) {
							gui_toggle_window_visibility (appGUI);
						}
                    } else {
                        if (strlen(gtk_entry_get_text(GTK_ENTRY(appGUI->nte->notes_find_entry)))) {
                            gtk_entry_set_text(GTK_ENTRY(appGUI->nte->notes_find_entry), "");
                        } else {
							if (config.enable_systray == TRUE && appGUI->no_tray == FALSE && appGUI->calendar_only == FALSE) {
								gui_toggle_window_visibility (appGUI);
							}
						}
                    }
                    return TRUE;
                case GDK_l:
                    if (event->state & GDK_CONTROL_MASK) {  /* CTRL + l */
                        gtk_widget_grab_focus(GTK_WIDGET(appGUI->nte->notes_find_entry));
                        return TRUE;
                    }
                    return FALSE;
            }

        } else {

            /* EDITOR */
            switch (event->keyval) {

                case GDK_w:
                    if (event->state & GDK_CONTROL_MASK) {  /* CTRL + w */
                        g_signal_emit_by_name(G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/close")), "clicked");
                        return TRUE;
                    }
                    return FALSE;

                case GDK_Escape:
					if (appGUI->nte->find_hbox_visible == FALSE) {
	                    g_signal_emit_by_name(G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/close")), "clicked");
                        return TRUE;
					}
                    return FALSE;

                case GDK_b:
                    if ((event->state & GDK_CONTROL_MASK) && !appGUI->nte->note_read_only) {  /* CTRL + b */
                        g_signal_emit_by_name(G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/bold")), "clicked");
                        return TRUE;
                    }
                    return FALSE;

                case GDK_i:
                    if ((event->state & GDK_CONTROL_MASK) && !appGUI->nte->note_read_only) {  /* CTRL + i */
                        g_signal_emit_by_name(G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/italic")), "clicked");
                        return TRUE;
                    }
                    return FALSE;

                case GDK_m:
                    if ((event->state & GDK_CONTROL_MASK) && !appGUI->nte->note_read_only) {  /* CTRL + m */
                        g_signal_emit_by_name(G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/mark_color")), "clicked");
                        return TRUE;
                    }
                    return FALSE;

                case GDK_u:
                    if ((event->state & GDK_CONTROL_MASK) && !appGUI->nte->note_read_only) {  /* CTRL + u */
                        g_signal_emit_by_name(G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/underline")), "clicked");
                        return TRUE;
                    }
                    return FALSE;

                case GDK_t:
                    if ((event->state & GDK_CONTROL_MASK) && !appGUI->nte->note_read_only) {  /* CTRL + t */
                        g_signal_emit_by_name(G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/strike")), "clicked");
                        return TRUE;
                    }
                    return FALSE;

                case GDK_s:
                    if ((event->state & GDK_CONTROL_MASK) && !appGUI->nte->note_read_only) {  /* CTRL + s */
                        g_signal_emit_by_name(G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/save")), "clicked");
                        return TRUE;
                    }
                    return FALSE;

                case GDK_f:
                    if (event->state & GDK_CONTROL_MASK) {  /* CTRL + f */
                        g_signal_emit_by_name(G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/find")), "clicked");
                        return TRUE;
                    }
                    return FALSE;

                case GDK_n:
                    if ((event->state & GDK_CONTROL_MASK) && !appGUI->nte->note_read_only) {  /* CTRL + n */
                        g_signal_emit_by_name(G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/clear")), "clicked");
                        return TRUE;
                    }
                    return FALSE;
            }
        }
    }
#endif  /* NOTES_ENABLED */

    /************************************************************************/
    /*** GLOBAL SHORTCUTS                                                 ***/
    /************************************************************************/

    switch (event->keyval) {

		case GDK_Escape:
			if (config.enable_systray == TRUE && appGUI->no_tray == FALSE && appGUI->calendar_only == FALSE) {
				gui_toggle_window_visibility (appGUI);
			}
			return FALSE;

        case GDK_q:
            if (event->state & GDK_CONTROL_MASK) {  /* CTRL + Q */
                gui_quit_osmo (appGUI);
            }
            return FALSE;

        case GDK_Page_Up:
#ifdef NOTES_ENABLED
            if (appGUI->nte->editor_active == FALSE) {
#endif  /* NOTES_ENABLED */
                if (event->state & GDK_CONTROL_MASK) {  /* CTRL + Page_Up */
                    set_visible_page(appGUI->current_tab, FALSE, appGUI);
                    return TRUE;
                } else if (page == PAGE_CALENDAR) {
                    if (config.day_notes_visible) {
                        if (event->state & GDK_MOD1_MASK) {  /* ALT + Page_Up */
                            calendar_btn_prev_month(appGUI);
                            return TRUE;
                        }
                    } else {
                        calendar_btn_prev_month(appGUI);
                        return TRUE;
                    }
                }
#ifdef NOTES_ENABLED
            }
#endif  /* NOTES_ENABLED */
            return FALSE;

        case GDK_Page_Down:
#ifdef NOTES_ENABLED
            if (appGUI->nte->editor_active == FALSE) {
#endif  /* NOTES_ENABLED */
                if (event->state & GDK_CONTROL_MASK) {  /* CTRL + Page_Down */
                    set_visible_page(appGUI->current_tab, TRUE, appGUI);
                    return TRUE;
                } else if (page == PAGE_CALENDAR) {
                    if (config.day_notes_visible) {
                        if (event->state & GDK_MOD1_MASK) {  /* ALT + Page_Down */
                            calendar_btn_next_month(appGUI);
                            return TRUE;
                        }
                    } else {
                        calendar_btn_next_month(appGUI);
                        return TRUE;
                    }
                }
#ifdef NOTES_ENABLED
            }
#endif  /* NOTES_ENABLED */
            return FALSE;
        case GDK_F1:
            select_tab (0, appGUI);
            return TRUE;
        case GDK_1:
            if ((event->state & GDK_MOD1_MASK)) {  /* ALT + 1 */
#if defined(TASKS_ENABLED) || defined(CONTACTS_ENABLED) || defined(NOTES_ENABLED)
                if (event->state & GDK_CONTROL_MASK) {  /* CTRL + ALT + 1 */
                    g_signal_emit_by_name(G_OBJECT(appGUI->opt->hide_calendar_checkbutton), "clicked");
                    select_tab (0, appGUI);
                } else {
#endif
                    select_tab (0, appGUI);
#if defined(TASKS_ENABLED) || defined(CONTACTS_ENABLED) || defined(NOTES_ENABLED)
                }
#endif
                return TRUE;
            }
            return FALSE;
        case GDK_F2:
            select_tab (1, appGUI);
            return TRUE;
        case GDK_2:
            if ((event->state & GDK_MOD1_MASK)) {  /* ALT + 2 */
#ifdef TASKS_ENABLED
                if (event->state & GDK_CONTROL_MASK) {  /* CTRL + ALT + 2 */
                    g_signal_emit_by_name(G_OBJECT(appGUI->opt->hide_tasks_checkbutton), "clicked");
                    select_tab (0, appGUI);
                } else {
#endif  /* TASKS_ENABLED */
                    select_tab (1, appGUI);
#ifdef TASKS_ENABLED
                }
#endif  /* TASKS_ENABLED */
                return TRUE;
            }
            return FALSE;
        case GDK_F3:
            select_tab (2, appGUI);
            return TRUE;
        case GDK_3:
            if ((event->state & GDK_MOD1_MASK)) {  /* ALT + 3 */
#ifdef CONTACTS_ENABLED
                if (event->state & GDK_CONTROL_MASK) {  /* CTRL + ALT + 3 */
                    g_signal_emit_by_name(G_OBJECT(appGUI->opt->hide_contacts_checkbutton), "clicked");
                    select_tab (0, appGUI);
                } else {
#endif  /* CONTACTS_ENABLED */
                    select_tab (2, appGUI);
#ifdef CONTACTS_ENABLED
                }
#endif  /* CONTACTS_ENABLED */
                return TRUE;
            }
            return FALSE;
        case GDK_F4:
            select_tab (3, appGUI);
            return TRUE;
        case GDK_4:
            if ((event->state & GDK_MOD1_MASK)) {  /* ALT + 4 */
#ifdef NOTES_ENABLED
                if (event->state & GDK_CONTROL_MASK) {  /* CTRL + ALT + 4 */
                    g_signal_emit_by_name(G_OBJECT(appGUI->opt->hide_notes_checkbutton), "clicked");
                    select_tab (0, appGUI);
                } else {
#endif  /* NOTES_ENABLED */
                   select_tab (3, appGUI);
#ifdef NOTES_ENABLED
                }
#endif  /* NOTES_ENABLED */
                return TRUE;
            }
            return FALSE;
        case GDK_F5:
			gtk_widget_show (GTK_WIDGET(opt_create_preferences_window (appGUI)));
            return TRUE;
        case GDK_F6:
			gtk_widget_show (GTK_WIDGET(opt_create_about_window (appGUI)));
            return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
notebook_sw_cb (GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, gpointer user_data) {
 
    GUI *appGUI = (GUI *)user_data;

#ifdef NOTES_ENABLED
	if (appGUI->nte->editor_active == TRUE) {
		g_signal_emit_by_name(G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/close")), "clicked");
	}
#endif  /* NOTES_ENABLED */

	appGUI->current_tab = page_num;
}

/*------------------------------------------------------------------------------*/

gboolean            
main_window_resized_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    if (appGUI->calendar_only == FALSE) {

#ifdef TASKS_ENABLED
        if (appGUI->tsk->tasks_panel_status == FALSE && !config.gui_layout) {
            gtk_paned_set_position(GTK_PANED(appGUI->tsk->tasks_paned), 99999);
        }
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
        if (appGUI->cnt->contacts_panel_status == FALSE && !config.gui_layout) {
            gtk_paned_set_position(GTK_PANED(appGUI->cnt->contacts_paned), 99999);
        }
#endif  /* CONTACTS_ENABLED */
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

#if (GTK_CHECK_VERSION(2,16,0))
gboolean
trayicon_clicked_cb (GtkStatusIcon *status_icon, GdkEventButton *event, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

	if (event->button == 1) {   /* LMB */

		gtk_status_icon_set_blinking (appGUI->osmo_trayicon, FALSE);
	    gui_toggle_window_visibility (appGUI);
		return TRUE;

	} else if (event->button == 2) {    /* MMB */

		gtk_status_icon_set_blinking (appGUI->osmo_trayicon, FALSE);
        gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_NORMAL);
		return TRUE;
	}

	return FALSE;
}
#else
void 
trayicon_clicked_cb (GtkStatusIcon *status_icon, gpointer user_data) {

	GUI *appGUI = (GUI *)user_data;

	gtk_status_icon_set_blinking (appGUI->osmo_trayicon, FALSE);
	gui_toggle_window_visibility (appGUI);
}
#endif /* GTK_CHECK_VERSION */

/*------------------------------------------------------------------------------*/

void 
trayicon_popup_cb (GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    gtk_menu_popup(GTK_MENU(appGUI->trayicon_popup_menu), NULL, NULL, NULL, NULL, 
                   button, activate_time);
}

void
systray_popup_menu_quit_selected_cb (gpointer user_data) {
    GUI *appGUI = (GUI *)user_data;
    gui_quit_osmo (appGUI);
}

void
systray_popup_menu_show_calendar_selected_cb (gpointer user_data) {
    GUI *appGUI = (GUI *)user_data;
    if (appGUI->window_visible == FALSE) {
        gtk_widget_show (appGUI->main_window);
        appGUI->window_visible = TRUE;
    }
    gtk_window_deiconify (GTK_WINDOW(appGUI->main_window));
    gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_NORMAL);
	gtk_status_icon_set_blinking (appGUI->osmo_trayicon, FALSE);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (appGUI->notebook), PAGE_CALENDAR);
}

#ifdef TASKS_ENABLED
void
systray_popup_menu_show_tasks_selected_cb (gpointer user_data) {
    GUI *appGUI = (GUI *)user_data;
    if (appGUI->window_visible == FALSE) {
        gtk_widget_show (appGUI->main_window);
        appGUI->window_visible = TRUE;
    }
    gtk_window_deiconify (GTK_WINDOW(appGUI->main_window));
    gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_NORMAL);
	gtk_status_icon_set_blinking (appGUI->osmo_trayicon, FALSE);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (appGUI->notebook), PAGE_TASKS);
}
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
void
systray_popup_menu_show_contacts_selected_cb (gpointer user_data) {
    GUI *appGUI = (GUI *)user_data;
    if (appGUI->window_visible == FALSE) {
        gtk_widget_show (appGUI->main_window);
        appGUI->window_visible = TRUE;
    }
    gtk_window_deiconify (GTK_WINDOW(appGUI->main_window));
    gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_NORMAL);
	gtk_status_icon_set_blinking (appGUI->osmo_trayicon, FALSE);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (appGUI->notebook), PAGE_CONTACTS);
}
#endif  /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
void
systray_popup_menu_show_notes_selected_cb (gpointer user_data) {
    GUI *appGUI = (GUI *)user_data;
    if (appGUI->window_visible == FALSE) {
        gtk_widget_show (appGUI->main_window);
        appGUI->window_visible = TRUE;
    }
    gtk_window_deiconify (GTK_WINDOW(appGUI->main_window));
    gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_NORMAL);
	gtk_status_icon_set_blinking (appGUI->osmo_trayicon, FALSE);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (appGUI->notebook), PAGE_NOTES);
}
#endif  /* NOTES_ENABLED */

void
systray_popup_menu_show_options_selected_cb (GUI *appGUI)
{
	if (appGUI->window_visible == FALSE) {
		gtk_widget_show (appGUI->main_window);
		appGUI->window_visible = TRUE;
	}
	gtk_window_deiconify (GTK_WINDOW (appGUI->main_window));
	gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_NORMAL);
	gtk_status_icon_set_blinking (appGUI->osmo_trayicon, FALSE);
	gtk_widget_show (opt_create_preferences_window (appGUI));
}

/*------------------------------------------------------------------------------*/

void
gui_systray_tooltip_update (GUI *appGUI)
{
	gchar *tstr, *dstr, *text;

	if (appGUI->osmo_trayicon == NULL) return;

	dstr = utl_date_print_j (utl_date_get_current_julian (), DATE_FULL,
	                         config.override_locale_settings);

	tstr = utl_time_print_default (utl_time_get_current_seconds (), FALSE);

	text = g_strdup_printf ("%s, %s", dstr, tstr);
	gtk_status_icon_set_tooltip (appGUI->osmo_trayicon, text);

	g_free (tstr);
	g_free (dstr);
	g_free (text);
}

/*------------------------------------------------------------------------------*/

void
gui_systray_update_icon (GUI *appGUI) {

guint32 julian_day;
gboolean flag = FALSE;
        
	if (appGUI->window_visible == TRUE) return;

    julian_day = utl_date_get_current_julian ();

	if (config.ignore_day_note_events == FALSE) {
		if (cal_check_note (julian_day, appGUI) == TRUE) {
			gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_NOTE);
			flag = TRUE;
		}
	}

#ifdef TASKS_ENABLED
    if (tsk_check_tasks (julian_day, julian_day, STATE_NONE, appGUI)) {
        gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_TASK);
		flag = TRUE;
    }
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
    if (check_contacts(julian_day, appGUI) == TRUE) {
        gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_BIRTHDAY);
		flag = TRUE;
    }
#endif  /* CONTACTS_ENABLED */

	if (flag == FALSE) {
        gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_NORMAL);
	} else {
		if (config.blink_on_events) {
			gtk_status_icon_set_blinking (appGUI->osmo_trayicon, TRUE);
		}
	}
}

/*------------------------------------------------------------------------------*/

void
gui_systray_initialize (GUI *appGUI) {

GtkWidget   *menu_entry;
GtkWidget   *systray_menu_separator;

    if (appGUI->calendar_only == TRUE) {
        return;
    }

    appGUI->trayicon_popup_menu = gtk_menu_new();

    appGUI->trayicon_menu_calendar_item = gtk_image_menu_item_new_with_label (_("Show calendar"));
    gtk_menu_shell_append (GTK_MENU_SHELL(appGUI->trayicon_popup_menu), appGUI->trayicon_menu_calendar_item);
    g_signal_connect_swapped (G_OBJECT(appGUI->trayicon_menu_calendar_item), "activate", 
                              G_CALLBACK(systray_popup_menu_show_calendar_selected_cb), appGUI);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (appGUI->trayicon_menu_calendar_item), 
                                   gtk_image_new_from_stock(OSMO_STOCK_SYSTRAY_MENU_CALENDAR, GTK_ICON_SIZE_MENU));
	if (config.hide_calendar == FALSE) {
        gtk_widget_show (appGUI->trayicon_menu_calendar_item);
	}

#ifdef TASKS_ENABLED
    appGUI->trayicon_menu_tasks_item = gtk_image_menu_item_new_with_label (_("Show tasks"));
    gtk_menu_shell_append (GTK_MENU_SHELL(appGUI->trayicon_popup_menu), appGUI->trayicon_menu_tasks_item);
    g_signal_connect_swapped (G_OBJECT(appGUI->trayicon_menu_tasks_item), "activate", 
                              G_CALLBACK(systray_popup_menu_show_tasks_selected_cb), appGUI);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (appGUI->trayicon_menu_tasks_item), 
                                   gtk_image_new_from_stock(OSMO_STOCK_SYSTRAY_MENU_TASKS, GTK_ICON_SIZE_MENU));
    
	if (config.hide_tasks == FALSE) {
	    gtk_widget_show (appGUI->trayicon_menu_tasks_item);
	}
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
    appGUI->trayicon_menu_contacts_item = gtk_image_menu_item_new_with_label (_("Show contacts"));
    gtk_menu_shell_append (GTK_MENU_SHELL(appGUI->trayicon_popup_menu), appGUI->trayicon_menu_contacts_item);
    g_signal_connect_swapped (G_OBJECT(appGUI->trayicon_menu_contacts_item), "activate", 
                              G_CALLBACK(systray_popup_menu_show_contacts_selected_cb), appGUI);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (appGUI->trayicon_menu_contacts_item), 
                                   gtk_image_new_from_stock(OSMO_STOCK_SYSTRAY_MENU_CONTACTS, GTK_ICON_SIZE_MENU));
	if (config.hide_contacts == FALSE) {
        gtk_widget_show (appGUI->trayicon_menu_contacts_item);
	}
#endif  /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
    appGUI->trayicon_menu_notes_item = gtk_image_menu_item_new_with_label (_("Show notes"));
    gtk_menu_shell_append (GTK_MENU_SHELL(appGUI->trayicon_popup_menu), appGUI->trayicon_menu_notes_item);
    g_signal_connect_swapped (G_OBJECT(appGUI->trayicon_menu_notes_item), "activate", 
                              G_CALLBACK(systray_popup_menu_show_notes_selected_cb), appGUI);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (appGUI->trayicon_menu_notes_item), 
                                   gtk_image_new_from_stock(OSMO_STOCK_SYSTRAY_MENU_NOTES, GTK_ICON_SIZE_MENU));
	if (config.hide_notes == FALSE) {
        gtk_widget_show (appGUI->trayicon_menu_notes_item);
	}
#endif  /* NOTES_ENABLED */

    systray_menu_separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(appGUI->trayicon_popup_menu), systray_menu_separator);
    gtk_widget_show(systray_menu_separator);

	menu_entry = gtk_image_menu_item_new_with_label (_("Show options"));
    gtk_menu_shell_append(GTK_MENU_SHELL(appGUI->trayicon_popup_menu), menu_entry);
    g_signal_connect_swapped(G_OBJECT(menu_entry), "activate", 
                             G_CALLBACK(systray_popup_menu_show_options_selected_cb), appGUI);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_entry), 
                                   gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));
    gtk_widget_show(menu_entry);

    systray_menu_separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(appGUI->trayicon_popup_menu), systray_menu_separator);
    gtk_widget_show(systray_menu_separator);

	menu_entry = gtk_image_menu_item_new_with_label (_("Quit"));
    gtk_menu_shell_append(GTK_MENU_SHELL(appGUI->trayicon_popup_menu), menu_entry);
    g_signal_connect_swapped(G_OBJECT(menu_entry), "activate", 
                             G_CALLBACK(systray_popup_menu_quit_selected_cb), appGUI);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_entry), 
                                   gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU));
    gtk_widget_show(menu_entry);

    /* create tray icon */

    appGUI->osmo_trayicon = gtk_status_icon_new_from_stock(OSMO_STOCK_SYSTRAY_NORMAL);
#if (GTK_CHECK_VERSION(2,16,0))
    g_signal_connect(G_OBJECT(appGUI->osmo_trayicon), "button-release-event", 
                     G_CALLBACK(trayicon_clicked_cb), appGUI);
#else
    g_signal_connect(G_OBJECT(appGUI->osmo_trayicon), "activate", 
                     G_CALLBACK(trayicon_clicked_cb), appGUI);
#endif /* GTK_CHECK_VERSION */

    g_signal_connect(G_OBJECT(appGUI->osmo_trayicon), "popup-menu",
                     G_CALLBACK(trayicon_popup_cb), appGUI);

    appGUI->window_visible = TRUE;

    if (config.enable_systray == TRUE) {
        gtk_status_icon_set_visible (appGUI->osmo_trayicon, TRUE);
        while (g_main_context_iteration (NULL, FALSE));

        if (gtk_status_icon_is_embedded (appGUI->osmo_trayicon) == FALSE) {
            appGUI->no_tray = TRUE;
            gtk_status_icon_set_visible (appGUI->osmo_trayicon, FALSE);
            gtk_widget_show(appGUI->main_window);
        } else {
            appGUI->no_tray = FALSE;
            gtk_status_icon_set_visible (appGUI->osmo_trayicon, TRUE);
            if (config.start_minimised_in_systray) {
                appGUI->window_visible = FALSE;
            } else {
                gtk_widget_show (appGUI->main_window);
            }
        }
    } else {
        gtk_status_icon_set_visible (appGUI->osmo_trayicon, FALSE);
        gtk_widget_show (appGUI->main_window);
    }

	gui_systray_update_icon (appGUI);
    gui_systray_tooltip_update (appGUI);
}

/*------------------------------------------------------------------------------*/

gboolean
gui_create_window (GUI *appGUI)
{
	GdkScreen *screen;
	gint sw, sh;
	GdkPixbuf *icon;
	gchar tmpbuf[BUFFER_SIZE];

#ifdef CONTACTS_ENABLED
    appGUI->cnt->contacts_filter_disabled = TRUE;
#endif  /* CONTACTS_ENABLED */
    appGUI->all_pages_added = FALSE;

    appGUI->cal->fd_day_name_font = pango_font_description_from_string(config.day_name_font);
    appGUI->cal->fd_cal_font = pango_font_description_from_string(config.calendar_font);
    appGUI->cal->fd_notes_font = pango_font_description_from_string(config.notes_font);

    appGUI->main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	g_snprintf (tmpbuf, BUFFER_SIZE, "Osmo %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
    gtk_window_set_title (GTK_WINDOW (appGUI->main_window), tmpbuf);

   	gtk_widget_set_events(appGUI->main_window, GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

    icon = gdk_pixbuf_new_from_inline(-1, osmo_icon, FALSE, NULL);
    gtk_window_set_icon (GTK_WINDOW(appGUI->main_window), icon);
    g_object_unref(icon);

	screen = gdk_screen_get_default ();
	if (screen != NULL) {
		sw = gdk_screen_get_width (screen);
		sh = gdk_screen_get_height (screen);
		if (config.window_x >= sw || config.window_x < 0)
			config.window_x = 0;
		if (config.window_y >= sh || config.window_y < 0)
			config.window_y = 0;
	}

    gtk_window_move (GTK_WINDOW (appGUI->main_window), config.window_x, config.window_y);

    gtk_widget_realize (appGUI->main_window);

    if (appGUI->calendar_only == FALSE) {
        gtk_window_set_default_size (GTK_WINDOW(appGUI->main_window), config.window_size_x, config.window_size_y);
    } else {
        gtk_window_set_default_size (GTK_WINDOW(appGUI->main_window), 500, -1);
    }
    gtk_window_set_resizable (GTK_WINDOW (appGUI->main_window), TRUE);

    g_signal_connect (G_OBJECT (appGUI->main_window), "delete-event",
                      G_CALLBACK(gui_window_close_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->main_window), "key_press_event",
                      G_CALLBACK (key_press_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->main_window), "configure_event",
                      G_CALLBACK (main_window_resized_cb), appGUI);

    osmo_register_stock_icons ();
    utl_gui_url_initialize (appGUI);

    appGUI->notebook = gtk_notebook_new ();
    GTK_WIDGET_UNSET_FLAGS (appGUI->notebook, GTK_CAN_FOCUS);
    gtk_widget_show (appGUI->notebook);
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK(appGUI->notebook), config.tabs_position);
    gtk_container_add (GTK_CONTAINER(appGUI->main_window), appGUI->notebook);
    g_signal_connect (G_OBJECT(appGUI->notebook), "switch-page", 
                      G_CALLBACK(notebook_sw_cb), appGUI);

    gtk_notebook_set_scrollable (GTK_NOTEBOOK(appGUI->notebook), appGUI->tiny_gui);

    if (appGUI->calendar_only == TRUE) {
        gtk_notebook_set_show_tabs (GTK_NOTEBOOK(appGUI->notebook), FALSE);
    }

	appGUI->opt->calendar_ical_files_store =
	    gtk_list_store_new (ICAL_NUMBER_OF_COLUMNS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING,
	                        G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
							G_TYPE_BOOLEAN);
	appGUI->opt->calendar_category_store = gtk_list_store_new (3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);

    gui_create_calendar(appGUI->notebook, appGUI);

    if (appGUI->calendar_only == FALSE) {

#ifdef TASKS_ENABLED
		appGUI->opt->tasks_category_store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);
        gui_create_tasks (appGUI);
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
		appGUI->opt->contacts_group_store = gtk_list_store_new (1, G_TYPE_STRING);
        gui_create_contacts (appGUI);
#endif  /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
        gui_create_notes (appGUI);
#endif  /* NOTES_ENABLED */

#ifdef HAVE_GTKSPELL
	if (config.day_note_spell_checker == TRUE) {
		GtkSpell *edSpell = gtkspell_new_attach (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview), NULL, NULL);
		if (config.override_locale_settings == TRUE) {
			gtkspell_set_language (edSpell, config.spell_lang, NULL);
		} else {
			gtkspell_set_language (edSpell, g_getenv ("LANG"), NULL);
		}
	}
#endif  /* HAVE_GTKSPELL */

	if (!config.gui_layout) {
		if (config.enable_auxilary_calendars == TRUE) {
			gtk_widget_show (appGUI->cal->aux_cal_expander);
		} else {
			gtk_widget_hide (appGUI->cal->aux_cal_expander);
		}
	}

        cal_read_notes (appGUI);

#ifdef HAVE_LIBICAL
        ics_initialize_timezone ();
        read_ical_entries (appGUI);
#endif  /* HAVE_LIBICAL */

#ifdef TASKS_ENABLED
        read_tasks_entries (appGUI);
        apply_task_attributes (appGUI);
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
        read_contacts_entries (appGUI);
        set_export_active (appGUI);
#endif  /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
		appGUI->opt->notes_category_store = gtk_list_store_new (1, G_TYPE_STRING);
        read_notes_entries (appGUI);

        utl_gui_create_category_combobox (GTK_COMBO_BOX (appGUI->nte->cf_combobox), 
										  appGUI->opt->notes_category_store, FALSE);

        if (config.remember_category_in_notes == TRUE) {
            gtk_combo_box_set_active (GTK_COMBO_BOX(appGUI->nte->cf_combobox), config.current_category_in_notes);
        } else {
            gtk_combo_box_set_active (GTK_COMBO_BOX(appGUI->nte->cf_combobox), 0);
        }
#endif  /* NOTES_ENABLED */

#ifdef TASKS_ENABLED
        refresh_tasks (appGUI);
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
        appGUI->cnt->contacts_filter_disabled = FALSE;
#endif  /* CONTACTS_ENABLED */

        appGUI->number_of_tabs = NUMBER_OF_TABS;

        if(config.remember_latest_tab == TRUE) {
            appGUI->current_tab = config.latest_tab;
        } else {
            appGUI->current_tab = PAGE_CALENDAR;
        }
    } else {
        appGUI->number_of_tabs = 1;
        appGUI->current_tab = 0;
    }

    appGUI->all_pages_added = TRUE;
    calendar_set_today (appGUI);

    update_aux_calendars (appGUI);

    if (appGUI->check_events == TRUE) {
        return create_event_checker_window (appGUI);
    } else {
        gui_systray_initialize (appGUI);
    }
 
    if (appGUI->calendar_only == TRUE) {
        gtk_widget_show (appGUI->main_window);
    }

    gtk_notebook_set_current_page(GTK_NOTEBOOK(appGUI->notebook), appGUI->current_tab);

    return TRUE;
}

/*------------------------------------------------------------------------------*/

void
gui_add_to_notebook (GtkWidget *widget, gchar *text, GUI *appGUI) {

GtkWidget *label;

    label = gtk_label_new(NULL);

    if (config.tabs_position == GTK_POS_LEFT) {
        gtk_label_set_angle (GTK_LABEL(label), 90.0);
    } else if (config.tabs_position == GTK_POS_RIGHT) {
        gtk_label_set_angle (GTK_LABEL(label), -90.0);
    }
    gtk_label_set_markup (GTK_LABEL (label), text);

    gtk_notebook_append_page(GTK_NOTEBOOK(appGUI->notebook), widget, label);

}

/*------------------------------------------------------------------------------*/

