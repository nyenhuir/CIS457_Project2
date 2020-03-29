/*
Reuben Nyenhuis
CIS457 Project 2
Due 3/30/2020
This project is designed to be a peer to peer chat application.
Two users run the program and communicate with each other, while using emojis.
gcc -o fulltestA Project2A.c `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` -lpthread
*/

#include <gtk/gtk.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>

//Global variables used for the GUI
GtkWidget *window;
GtkWidget *fixed;
GtkWidget *entry;
GtkWidget *check;
GtkWidget *label;
GtkWidget *emojibutton;
GtkWidget *dialog;
GtkDialogFlags diaflags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
GtkWidget *connectbutton;
GtkWidget *textArea;
GtkTextBuffer *buffer;
GtkWidget *scrolledWindow;
GtkAdjustment *vadj;
GtkTextChildAnchor* anchor;
GtkTextMark *mark;
GtkTextIter iter;
GtkWidget *image;
int temp=0;
int run;
char *entry_text;


//Global variables for socket communications
int sockfd, portno, n, needconnection;
struct sockaddr_in serv_addr;
char bufferthr[4096];
pthread_t thread1,thread2;
int status;
char str[255];
struct sockaddr_in serv_addr;
struct hostent *servercli;




//Prototype for server thread functions
void *server(void *arg); 

//Shows the dialog for emojis
static void show_dialog( GtkWidget *widget, GtkWidget *emojibutton){
  //If the connection hasn't been made no text can be sent.
  if(needconnection) return;

  //Gets the text from the user input and saves it so it can send with emoji.
  entry_text = gtk_entry_get_text (GTK_ENTRY (entry));
  dialog = gtk_dialog_new_with_buttons ("Emoji Options", window, diaflags,
                                       ("smile"),1,("laugh"),2,("unsure"),3,NULL);
  //variable to retrieve dialog option selected
  int result = gtk_dialog_run (GTK_DIALOG (dialog));

  //switch used for the different emoji options
  //Each option writes the correct output to the screen and sends it to the peer
  switch (result)
    {
      case 1:
	 gtk_text_buffer_insert_at_cursor(buffer, "Me: ", -1);
 	 gtk_text_buffer_insert_at_cursor(buffer, entry_text, -1);
	 bzero(bufferthr,4096);
 	 bufferthr[0]='A';
	 strcat(bufferthr,entry_text);
  	 n = write(sockfd,bufferthr,strlen(bufferthr));
  	 if (n < 0) 
       		perror("ERROR writing to socket");

         image = gtk_image_new_from_file("smile.PNG");
         gtk_container_add (GTK_CONTAINER (scrolledWindow),image);
         gtk_widget_show (image);
	 bzero(bufferthr,4096);
 	 gtk_text_buffer_get_end_iter(buffer,&iter);
 	 anchor = gtk_text_buffer_create_child_anchor(buffer, &iter);
 	 gtk_text_view_add_child_at_anchor( GTK_TEXT_VIEW(textArea),image,anchor);
	 gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
	 gtk_entry_set_text (GTK_ENTRY (entry),"");
         break;
      case 2:
	 gtk_text_buffer_insert_at_cursor(buffer, "Me: ", -1);
 	 gtk_text_buffer_insert_at_cursor(buffer, entry_text, -1);
	 bzero(bufferthr,4096);
 	 bufferthr[0]='B';
	 strcat(bufferthr,entry_text);
  	 n = write(sockfd,bufferthr,strlen(bufferthr));
  	 if (n < 0) 
       		perror("ERROR writing to socket");

         image = gtk_image_new_from_file("laugh.PNG");
         gtk_container_add (GTK_CONTAINER (scrolledWindow),image);
         gtk_widget_show (image);
 	 gtk_text_buffer_get_end_iter(buffer,&iter);
 	 anchor = gtk_text_buffer_create_child_anchor(buffer, &iter);
 	 gtk_text_view_add_child_at_anchor( GTK_TEXT_VIEW(textArea),image,anchor);
	 gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
	 gtk_entry_set_text (GTK_ENTRY (entry),"");
         break;
      case 3:
	 gtk_text_buffer_insert_at_cursor(buffer, "Me: ", -1);
 	 gtk_text_buffer_insert_at_cursor(buffer, entry_text, -1);
	 bzero(bufferthr,4096);
 	 bufferthr[0]='C';
	 strcat(bufferthr,entry_text);
  	 n = write(sockfd,bufferthr,strlen(bufferthr));
  	 if (n < 0) 
       		perror("ERROR writing to socket");

         image = gtk_image_new_from_file("unsure.PNG");
         gtk_container_add (GTK_CONTAINER (scrolledWindow),image);
         gtk_widget_show (image);
 	 gtk_text_buffer_get_end_iter(buffer,&iter);
 	 anchor = gtk_text_buffer_create_child_anchor(buffer, &iter);
 	 gtk_text_view_add_child_at_anchor( GTK_TEXT_VIEW(textArea),image,anchor);
	 gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
	 gtk_entry_set_text (GTK_ENTRY (entry),"");
         break;

      default:
         // dialog_was_cancelled ();
         break;
    }
  //Removes the dialog after selection has been made
  gtk_widget_destroy (dialog);
}


//This function is used when the user enters text to be sent
static void enter_callback( GtkWidget *widget, GtkWidget *entry ){
  //If the connection hasn't been made no text can be sent.
  if(needconnection) return;

  //Saves the data that has been written and writes it to the peer and screen
  entry_text = gtk_entry_get_text (GTK_ENTRY (entry));
  bzero(bufferthr,4096);
  bufferthr[0]='N';
  strcat(bufferthr,entry_text);
  n = write(sockfd,bufferthr,strlen(bufferthr));
  if (n < 0) 
       	perror("ERROR writing to socket");
  //Puts the data onto the screen
  updateTextview();
  //Clears the entry text field
  gtk_entry_set_text (GTK_ENTRY (entry),"");

}

static void make_connection( GtkWidget *widget, GtkWidget *connectbutton ){
  //Starts the thread for the server portion of the p2p application
 if ((status = pthread_create(&thread2, NULL, server, str)) != 0) {
     fprintf(stderr, "thread create error %d: %s\n", status, strerror(status));
     exit(1);
 }
 //Detaches the server thread
 pthread_detach(thread2);

 //Tries to achieve connection with server
  while(needconnection){
       portno = 6551;
       sockfd = socket(AF_INET, SOCK_STREAM, 0);
       if (sockfd < 0) 
	 perror("ERROR opening socket (servercli)");
       servercli = gethostbyname("localhost");
       if (servercli == NULL) 
	   fprintf(stderr,"ERROR, no such host\n");
	 
       bzero((char *) &serv_addr, sizeof(serv_addr));
       serv_addr.sin_family = AF_INET;
       bcopy((char *)servercli->h_addr,(char *)&serv_addr.sin_addr.s_addr,servercli->h_length);
       serv_addr.sin_port = htons(portno);
       //Throw error if connect attempt does not work
       if ((connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)) {
	   close(sockfd);
	   sleep(3);
       }
       else {
	   needconnection=0; //We are now connected
	   gtk_widget_set_sensitive (connectbutton, FALSE); //Makes connection button unclickable
       } 
  }
	gtk_text_buffer_set_text(buffer, "Connection Established\n\n", -1);
}

//This function puts the desired text onto the screen for the user
void updateTextview(){

  //Writing the desired text to the screen
  gtk_text_buffer_insert_at_cursor(buffer, "Me: ", -1);
  gtk_text_buffer_insert_at_cursor(buffer, entry_text, -1);
  gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);

  //Adjusting the window to scroll with the new text if it is past the screen
  vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolledWindow));
  gtk_adjustment_set_value(vadj, 0);
  gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(scrolledWindow), vadj); 

}


//Main function initializes the GUI widgets and runs them.
int main( int   argc, char *argv[] ) { 

  //initializing the use of gtk
    gtk_init (&argc, &argv);

    //sets the values for the client/server to allow for connection to be made
    needconnection=1;
    run = 1;
    
    //Creates a new window for the GUI
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (GTK_WIDGET (window), 1530, 750);
    gtk_window_set_title (GTK_WINDOW (window), "CoronaVirus Communication Center Location A");
    g_signal_connect (window, "destroy",G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect_swapped(window,"delete-event",G_CALLBACK(gtk_widget_destroy),window);

    //Sets up a fixed GUI to allow for easier widget manipulation
    fixed = gtk_fixed_new ();
    gtk_container_add (GTK_CONTAINER (window), fixed);
    gtk_widget_show (fixed);

    //Creates a text entry box where the user can enter their desired message
    entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (entry), 4096);
    g_signal_connect (entry, "activate", G_CALLBACK (enter_callback), entry);
    gtk_entry_set_width_chars (entry, 110);
    gtk_fixed_put (GTK_FIXED (fixed), entry, 250, 550);
    gtk_widget_show(entry);

    //Creates a label to inform user that the text box is for messages
    label = gtk_label_new("Send Message:");
    gtk_fixed_put (GTK_FIXED (fixed), label, 135, 555);
    gtk_widget_show (label);

    //Creates button to open an emoji dialog where the user can select an emoji to send
    emojibutton = gtk_button_new_with_label("Add emoji");
    g_signal_connect_swapped (emojibutton, "clicked",G_CALLBACK (show_dialog),emojibutton);
    gtk_fixed_put (GTK_FIXED (fixed), emojibutton, 1250, 550);
    gtk_widget_show (emojibutton);

    //Creates button to connect to server
    connectbutton = gtk_button_new_with_label ("Connect");
    g_signal_connect_swapped(connectbutton,"clicked",G_CALLBACK(make_connection),connectbutton);
    gtk_fixed_put (GTK_FIXED (fixed), connectbutton, 700, 600);
    gtk_widget_show (connectbutton);

    //Creates a text view where the conversation can be observed
    textArea = gtk_text_view_new();
    gtk_widget_set_size_request(textArea,1000,500);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textArea), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(textArea), FALSE);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textArea), GTK_WRAP_WORD);

    //Initializes a buffer to keep track of the strings of the textview
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textArea));

    //Creates a scrollable window for the textview to be located in to allow the conversation to scroll
    scrolledWindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),GTK_POLICY_AUTOMATIC, 
                                  GTK_POLICY_AUTOMATIC);

    gtk_container_add (GTK_CONTAINER (scrolledWindow),textArea);
    
    gtk_fixed_put (GTK_FIXED (fixed), scrolledWindow, 250, 10);

    gtk_widget_show (textArea);

    gtk_widget_show_all (window);

    //Runs the GUI application
    gtk_main();

    return 0;
}

//Thread to run the server portion of the application
void *server(void *arg){
  //Variable declaration for all of the socket communication
     int sockfds, newsockfd, clilen, pid,status,clientcon,n;
     struct sockaddr_in serv_addr, cli_addr;
     char bufferser[4096];
     char emojitype;
     char* bufferp;

     //Setting up the server
     sockfds = socket(AF_INET, SOCK_STREAM, 0);
     //Throws error if not a valid socket
     if (sockfds < 0) 
     {
        perror("ERROR opening socket (server)");
     }
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY; //inet_addr("192.168.110.7");
     serv_addr.sin_port = htons(6550); //Our port number
     if (bind(sockfds, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
     {
        perror("pding");
     }

     //Listening for a client 
     listen(sockfds,50);

     clilen = sizeof(cli_addr);

     run =1;
     clientcon=0;
     while (run) 
     {
       //If connection hasn't been established yet, sets up connection
        if(!clientcon)
	{
          newsockfd = accept(sockfds, 
          (struct sockaddr *) &cli_addr, (int *) &clilen);
          if (newsockfd < 0) 
	    {
              perror("ERROR on accept"); //Not connected
	    }
	  else 
	    {
              clientcon=1; //we are now connected
	    }
	}

  	
	//Clears the buffer and waits for input from the socket
        close(sockfds);
	bzero(bufferser,4096);
  	n = read(newsockfd,bufferser,4096);   
  	if (n < 0)
	  perror("ERROR reading from socket");

	//After input has been read, program proceeds as long as there is valid input
	if(strlen(bufferser)>0){
	  //The first character sent is the emoji character
	  emojitype = bufferser[0];

	  //The emoji character is removed and the input is displayed to user
	  bufferp = &bufferser;
	  bufferp++;
	  gtk_text_buffer_insert_at_cursor(buffer,"Friend: ", -1);
 	  gtk_text_buffer_insert_at_cursor(buffer,bufferp, -1);

	  //If there is an emoji character, it sets up what it is
	  if(emojitype != 'N'){
	    switch(emojitype){
	    case 'A':
	      image = gtk_image_new_from_file("smile.PNG");
	      break;
	    case 'B':
	      image = gtk_image_new_from_file("laugh.PNG");
	      break;
	    case 'C':
	      image = gtk_image_new_from_file("unsure.PNG");
	      break;
	    default: break;
	    }

	    //Displays the emoji if necessary
	    gtk_container_add (GTK_CONTAINER (scrolledWindow),image);
            gtk_widget_show (image);

 	    gtk_text_buffer_get_end_iter(buffer,&iter);
 	    anchor = gtk_text_buffer_create_child_anchor(buffer, &iter);
 	    gtk_text_view_add_child_at_anchor( GTK_TEXT_VIEW(textArea),image,anchor);
	  }
	  gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
	}
     }
     return 0; /* we never get here */
}
