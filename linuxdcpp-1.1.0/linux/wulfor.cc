/*
 * Copyright © 2004-2008 Jens Oknelid, paskharen@gmail.com
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <glib/gi18n.h>

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>

#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include <iostream>
#include <signal.h>

extern pthread_mutex_t count_mutex;
extern pthread_cond_t cond;
int Policy = 0;


void callBack(void* x, const std::string& a)
{
	std::cout << "Loading: " << a << std::endl;
}

void *main_fb(void *a);
void *main_app(void *a);

int main(int argc, char *argv[])
{
       pthread_t thread_id;
       int a;
       FILE *file = NULL;
       ssize_t ret = 0;
       size_t len = 0;
       char *line = NULL;
       
       if(argc != 2) {
           printf("\n Usage: <Executable> <Policy File with Path>\n");
           return 0;
       }

       file = fopen(argv[1], "r");
       if(!file) {
           printf("\n##################################################");
           printf("\n Warning: Policy file not found. Setting Policy to \"ALL\"\n");
           printf("##################################################\n");
       }
       else {
           while ((ret = getline(&line, &len, file)) != -1) {
               line[ret-1]='\0';
               if(*line == '#' || *line == ' ' || *line == '\0')
                   continue;
               const char * val = strtok(line, " ");
               val = strtok(NULL, " ");
               Policy = atoi(val);
               printf("\n##################################################");
               printf("\nPolicy = %d\n", Policy);
               printf("##################################################\n");
           }

           if (line)
               free(line);
       }


       //main_fb((void *)&a);
	// Initialize i18n support
	bindtextdomain("linuxdcpp", _DATADIR "/locale");
	textdomain("linuxdcpp");
	bind_textdomain_codeset("linuxdcpp", "UTF-8");

	// Check if profile is locked
	if (WulforUtil::profileIsLocked())
	{
		gtk_init(&argc, &argv);
		std::string message = _("Only one instance of LinuxDC++ is allowed per profile");

		GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message.c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		return -1;
	}

	// Start the DC++ client core
	dcpp::startup(callBack, NULL);

	dcpp::TimerManager::getInstance()->start();

	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);
        //Create Thread for handling FB stuff
#if 0
	glade_init();
	g_set_application_name(_("LinuxDC++"));

	signal(SIGPIPE, SIG_IGN);

	WulforSettingsManager::newInstance();
	WulforManager::start();
       printf("\n after start \n");
	gdk_threads_enter();
       printf("\n after enter \n");
	gtk_main();
       printf("\n after main \n");
	gdk_threads_leave();
       printf("\n after leave \n");
	WulforManager::stop();
	WulforSettingsManager::deleteInstance();

	std::cout << "Shutting down..." << std::endl;
	dcpp::shutdown();
#endif
        pthread_create(&thread_id, NULL, (void *)main_app, (void *)&a);
        main_fb((void *)&a);
        pthread_join(thread_id, NULL);
	return 0;
}

void* main_app(void *a)
{
        pthread_mutex_init(&count_mutex, NULL);
        pthread_cond_init (&cond, NULL);
       //sleep(40);
         pthread_mutex_lock(&count_mutex);
         pthread_cond_wait(&cond, &count_mutex);
         pthread_mutex_unlock(&count_mutex);


	glade_init();
	g_set_application_name(_("LinuxDC++"));

	signal(SIGPIPE, SIG_IGN);

	WulforSettingsManager::newInstance();
	WulforManager::start();
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	WulforManager::stop();
	WulforSettingsManager::deleteInstance();

	std::cout << "Shutting down..." << std::endl;
	dcpp::shutdown();
	return 0;
}

