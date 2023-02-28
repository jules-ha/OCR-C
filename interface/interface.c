#include <gtk/gtk.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <time.h>
#include "../grid_detection/grid_detection.h"
#include "../NeuralNets/ocrNN.h"
#include "../pretraitement/rotation.h"
#include "../pretraitement/filtre.h"

// Structure of the graphical user interface.
typedef struct UserInterface
{
    GtkWindow* window;              // Main window
    GtkButton* start_button;        // Start button
    GtkButton* left_rot_button;     
    GtkButton* right_rot_button;
    GtkButton* auto_rot_button;
    GtkButton* training_button;
    GtkButton* save_button;
    GtkStack* stack;
    GtkImage* sudoku_image;
    GtkFileChooser* file_chooser_button;
    GtkWidget* page0;
    GtkWidget* page1;
    GtkLabel* process_label;
    GtkEntry* savename_entry;
    GtkEntry* name_ntw_entry;
    GtkEntry* custom_ntw_entry;
    GtkSpinButton* epoch;
    GtkSpinButton* nodes;
    GtkFileChooser* dataset;
    GtkSpinner* spin_nn;
    GtkLabel* success;
    char* filename;
    int rotation;

} UserInterface;



//Load an image from the filename on the interface
void load_soduku_image(const char* filename, UserInterface* ui)
{
    GError** error = NULL; 
    //Create the pixbuf from the filename
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(filename,error);
    //Resize it
    SDL_Surface* tmp = IMG_Load(filename);
    SDL_Surface* res = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_RGB888, 0);
    SDL_FreeSurface(tmp);
    int w, h;
    if (res->w > 1000)
    {
        w = 1000;
        h = res->h * 1000 / res->w;
    }
    else
    {
        h = res->h;
        w = res->w;
    }
    pixbuf = gdk_pixbuf_scale_simple (pixbuf,w ,h ,GDK_INTERP_BILINEAR);
    SDL_FreeSurface(res);
    //Display on the interface
    //remove the old image
    //gtk_image_clear(ui->sudoku_image);
    gtk_image_set_from_pixbuf(ui->sudoku_image,pixbuf);
    //ui->filename = (char*)filename;
}

//Check each step of treatment
void on_start(GtkButton *button, UserInterface* ui)
{
  gtk_widget_set_sensitive(GTK_WIDGET(ui->start_button), FALSE);
  SDL_Surface *image = load_image(ui->filename);
  SDL_Surface *finite_lines = imageToFiniteLines(image);
  SDL_Surface *res = buildGrid(finite_lines, image);
  SDL_SaveBMP(res, "./data/resolved.bmp");
  ui->filename = "./data/resolved.bmp";
  load_soduku_image(ui->filename,ui);
  gtk_widget_set_sensitive(GTK_WIDGET(ui->save_button), TRUE);
  gtk_label_set_label(ui->process_label,"Grid resolution : Done");
}


void file_choose(GtkFileChooserButton *button, gpointer user_data)
{
  UserInterface* ui = (UserInterface*) user_data;
  //Get the filename of the image given by the user in the gtkfilechooser button
  gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ui->file_chooser_button));
  ui->filename = (char*)filename;
  load_soduku_image(filename,ui);
  //Update the process label
  gtk_label_set_label(ui->process_label,"Process : waiting for start");
  //Unable the start_button
  gtk_widget_set_sensitive(GTK_WIDGET(ui->start_button), TRUE);
}

void left_rotation(GtkButton* button , gpointer user_data)
{
    UserInterface* ui = (UserInterface*) user_data;
    SDL_Surface* s2 = load_image(ui->filename);
    // create empty surface with same dimensions as s2
    SDL_Surface* s1 = SDL_CreateRGBSurface(0, s2->w, s2->h, 32, 0, 0, 0, 0);
    ui->rotation += 5;
    rotation(s1, s2, ui->rotation);
    SDL_SaveBMP(s1, "./data/left_rotation.bmp");
    //ui->filename = "./data/left_rotation.bmp";
    load_soduku_image("./data/left_rotation.bmp",ui);
    SDL_FreeSurface(s1);
    SDL_FreeSurface(s2);
}

void right_rotation(GtkButton* button , gpointer user_data)
{
    UserInterface* ui = (UserInterface*) user_data;
    SDL_Surface* s2 = load_image(ui->filename);
    // create empty surface with same dimensions as s2
    SDL_Surface* s1 = SDL_CreateRGBSurface(0, s2->w, s2->h, 32, 0, 0, 0, 0);
    ui->rotation -= 5;
    rotation(s1, s2, ui->rotation);
    SDL_SaveBMP(s1, "./data/left_rotation.bmp");
    //ui->filename = "./data/left_rotation.bmp";
    load_soduku_image("./data/left_rotation.bmp",ui);
    SDL_FreeSurface(s1);
    SDL_FreeSurface(s2);
}


//core function of the auto rotation
void aut_rot(UserInterface* ui)
{
    SDL_Surface* image = load_image(ui->filename);
    SDL_Surface* image2 = load_image(ui->filename);
    SDL_Surface* image3 = load_image(ui->filename);
    SDL_Surface* finite_lines = imageToFiniteLines(image);
    Rotation_automatique(image2, finite_lines, image3);
    SDL_SaveBMP(image2, "./data/auto_rotation.bmp");
    ui->filename = "./data/auto_rotation.bmp";
    load_soduku_image(ui->filename,ui);
    SDL_FreeSurface(image);
    SDL_FreeSurface(image2);
    SDL_FreeSurface(image3);
    SDL_FreeSurface(finite_lines);
}
// callback function for the auto_rotation button
void auto_rotation(GtkButton* button, gpointer user_data)
{
  UserInterface* ui = (UserInterface*) user_data;
  aut_rot(ui);
  gtk_label_set_label(ui->process_label,"Auto-Rotate : Done");
}


void training(GtkWidget *widget, UserInterface* ui)
{
    GtkWidget* page = ui->page1;
    GtkStack* stack = ui->stack;
    gtk_stack_set_visible_child(stack, page);
    gtk_widget_show(page);

}

void back(GtkWidget *widget, UserInterface* ui)
{
    GtkWidget* page = ui->page0;
    GtkStack* stack = ui->stack;
    gtk_stack_set_visible_child(stack, page);
    gtk_widget_show(page);

}

void save(GtkButton* button, gpointer user_data)
{
    UserInterface* ui = (UserInterface*) user_data;
    //Get the name of the image from the gtk entry
    char *filename = (char*)gtk_entry_get_text(ui->savename_entry);
    strcat(filename, ".bmp");
    SDL_Surface* image = load_image(ui->filename);
    SDL_SaveBMP(image, filename);
    SDL_FreeSurface(image);
}

void train_ntw(GtkButton* button, gpointer user_data)
{
    UserInterface* ui = (UserInterface*) user_data;
    //activate the GtkSpinner
    gtk_widget_set_sensitive(GTK_WIDGET(ui->spin_nn), TRUE);
    //Get the name of the network
    const gchar* name = gtk_entry_get_text(ui->name_ntw_entry);
    int epoch = gtk_spin_button_get_value_as_int(ui->epoch);
    int nodes = gtk_spin_button_get_value_as_int(ui->nodes);
    gchar* dataset = gtk_file_chooser_get_filename(ui->dataset);
    struct nn* network = malloc(sizeof(struct nn));
    initNetwork(network, epoch, nodes);
    train(network, dataset);
    char path[100];
    sprintf(path, "./%s.dat", name);
    saveNet(network, path);
    gtk_widget_set_sensitive(GTK_WIDGET(ui->spin_nn), FALSE);
    gtk_label_set_label(ui->success, "Training successfull");
    free(network);
    //TODO
}

// Main function.
int main (int argc, char *argv[])
{
  
    // Initializes GTK.
    gtk_init(NULL, NULL);

    // Loads the UI description and builds the UI.
    // (Exits if an error occurs.)
    GtkBuilder* builder = gtk_builder_new();
    GError* error = NULL;
    if (gtk_builder_add_from_file(builder, "ui2.glade", &error) == 0)
    {
        g_printerr("Error loading file: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }
    
    // Gets the widgets.
    //Window
    GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));

    //Stack
    GtkStack* stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
    //Get the neural network page
    GtkWidget* page1 = gtk_stack_get_child_by_name(stack,"page1");
    //Get the main page
    GtkWidget* page0 = gtk_stack_get_child_by_name(stack,"page0");

    ////Of the main_page////
    //GtkButton
    GtkButton* start_button = GTK_BUTTON(gtk_builder_get_object(builder, "start_button"));
    //Disable the start button
    gtk_widget_set_sensitive(GTK_WIDGET(start_button), FALSE);
    GtkButton* save_button = GTK_BUTTON(gtk_builder_get_object(builder, "save_button"));
    //Disable the save button
    gtk_widget_set_sensitive(GTK_WIDGET(save_button), FALSE);
    //GtkButton* cancel_button = GTK_BUTTON(gtk_builder_get_object(builder, "cancel_button"));
    //GtkButton* remove_button = GTK_BUTTON(gtk_builder_get_object(builder, "remove_button"));
    //GtkButton* fullscreen_button = GTK_BUTTON(gtk_builder_get_object(builder, "fullscreen_button"));
    GtkButton* left_rot_button = GTK_BUTTON(gtk_builder_get_object(builder, "left_rot_button"));
    GtkButton* right_rot_button = GTK_BUTTON(gtk_builder_get_object(builder, "right_rot_button"));
    GtkButton* auto_rot_button = GTK_BUTTON(gtk_builder_get_object(builder, "auto_rot_button"));
    GtkButton* training_button =  GTK_BUTTON(gtk_builder_get_object(builder, "training_button"));
    GtkButton* start_training = GTK_BUTTON(gtk_builder_get_object(builder, "start_training"));

    //GtkImage
    GtkImage* sudoku_image = GTK_IMAGE(gtk_builder_get_object(builder,"sudoku_image"));

    GtkFileChooser* file_chooser_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder,"file_chooser_button"));

    GtkLabel* process_label = GTK_LABEL(gtk_builder_get_object(builder, "process_label"));

    GtkEntry* savename_entry = GTK_ENTRY(gtk_builder_get_object(builder, "name_file_enter"));

    /////////

    ////Of the Neural Network Page////
    GtkButton* back_button = GTK_BUTTON(gtk_builder_get_object(builder, "back_button"));
    GtkEntry* name_ntw_entry = GTK_ENTRY(gtk_builder_get_object(builder, "name_ntw_entry"));
    GtkEntry* custom_ntw_entry = GTK_ENTRY(gtk_builder_get_object(builder, "custom_ntw_entry"));
    GtkSpinButton* epoch = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "epoch"));
    GtkSpinButton* nodes = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "nodes"));
    GtkFileChooser* dataset = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "dataset"));
    GtkSpinner* spin_nn = GTK_SPINNER(gtk_builder_get_object(builder, "spin_nn"));
    GtkLabel* success = GTK_LABEL(gtk_builder_get_object(builder, "success"));
    
    //Change the background color
    //GdkColor color;
    //gdk_color_parse("#303030", &color);
    //gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);

    // Creates the "UserInterface" structure.
    UserInterface ui =
    {
          .window = window,
          .start_button = start_button,
          .left_rot_button = left_rot_button,
          .right_rot_button = right_rot_button,
          .auto_rot_button = auto_rot_button,
          .training_button = training_button,
          .save_button = save_button,
          .stack = stack,
          .sudoku_image = sudoku_image,
          .file_chooser_button = file_chooser_button,
          .page1 = page1,
          .page0 = page0,
          .process_label = process_label,
          .savename_entry = savename_entry,
          .name_ntw_entry = name_ntw_entry,
          .custom_ntw_entry = custom_ntw_entry,
          .epoch = epoch,
          .nodes = nodes,
          .dataset = dataset,
          .spin_nn = spin_nn,
          .success = success,
          .filename = NULL,
          .rotation = 0,
    };


    // Connects signal handlers.

    //main page//
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(start_button, "clicked", G_CALLBACK(on_start), &ui);
    //g_signal_connect(cancel_button, "clicked", G_CALLBACK(close_window), window);
    //g_signal_connect(fullscreen_button, "clicked", G_CALLBACK(fullscreen),NULL);
    //g_signal_connect(remove_button, "clicked", G_CALLBACK(remove_window), NULL);
    g_signal_connect(left_rot_button, "clicked", G_CALLBACK(left_rotation), &ui);
    g_signal_connect(right_rot_button, "clicked", G_CALLBACK(right_rotation), &ui);
    g_signal_connect(auto_rot_button, "clicked", G_CALLBACK(auto_rotation), &ui);
    g_signal_connect(training_button, "clicked", G_CALLBACK(training), &ui);
    g_signal_connect(save_button, "clicked", G_CALLBACK(save), &ui);
    g_signal_connect(file_chooser_button, "file-set", G_CALLBACK(file_choose), &ui);
    g_signal_connect(start_training, "clicked", G_CALLBACK(train_ntw), &ui);
    ////

    //Neural Network page//
    g_signal_connect(back_button, "clicked", G_CALLBACK(back), &ui);
    
    // Loads the CSS file
    GtkCssProvider *cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, "./style.css", NULL);
    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen,
                                              GTK_STYLE_PROVIDER(cssProvider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Runs the main loop.
    gtk_widget_show_all((GtkWidget*)window);
    gtk_main();

    // Exits.
    return 0;
}
