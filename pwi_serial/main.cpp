
#include <vector>
#include <mutex>
#include <queue>
#include <gtk/gtk.h>
#include <iostream>
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time
#include <string>  // string
#include <thread>
#include <fstream>
#include <algorithm>

#if defined(_WIN32) || defined(WIN32)
//    #include <gdk/gdkwin32.h>
#elif defined(__unix__) || defined(__unix) || defined(unix) || defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#else
#endif


#include "Tserial_event.h"

// ========================================================================================================================

    GtkBuilder  *gbuilder;
    GError      *gerror = NULL;

    GtkWidget   *gtk_window_1;
    GtkWidget   *gtk_button_execute;
    GtkWidget   *gtk_button_connection;
    GtkWidget   *gtk_button_saveLogs;
    GtkWidget   *gtk_entry_1;
    GtkWidget   *gtk_entry_2;
    GtkWidget   *gtk_label_1;
    GtkWidget   *gtk_text_view_1;

    Tserial_event *com;
    std::mutex mutexDataIn;
    std::mutex mutextimeQueue;
    std::mutex mutexOrdersToSend;
    std::mutex mutexFirstCommand;
    std::mutex mutexReceivedOrders;
    std::mutex mutexLogtx;
    std::string firstCommand;
    std::vector<std::string> logtxVector;
    std::vector<std::string> dataIn;
    std::pair<std::string,std::string> firstReceive;
    std::queue<std::pair<std::string,std::string>> receivedOrders; //struct w ktorym bedzie time string
    std::queue<std::string> timeQueue;
    std::queue<std::string> ordersToSend;
    bool sent= false;
    gboolean threadsWork = FALSE;
    int error = 0;

// ========================================================================================================================

void logtx (std::string str);
void parseString(std::string& stringToParse);
void sendCommand();
std::string getCurrentTimestamp();
gboolean checkOrders(gpointer data);
gboolean orderInterpreter(gpointer data);

// ========================================================================================================================

/* ======================================================== */
/* ===============  OnCharArrival     ===================== */
/* ======================================================== */
void onDataArrival(int size, char *buffer)
{
    std::lock_guard<std::mutex> guard(mutexDataIn);
    if ((size>0) && (buffer!=0))
    {

        buffer[size] = 0;
        //gtk_label_set_text(GTK_LABEL(gtk_label_1), buffer);
        dataIn.push_back(std::string(buffer));
        if(*buffer=='\r')
        {
            std::lock_guard<std::mutex> guard2(mutextimeQueue);
            timeQueue.push(getCurrentTimestamp());
        }
    }
}

/* ======================================================== */
/* ===============  OnCharArrival     ===================== */
/* ======================================================== */
void serialEventManager(void* object, uint32 event)    // BYLO: void serialEventManager(uint32 object, uint32 event)
{
    char *buffer;
    int   size;
    Tserial_event *com;

    com = (Tserial_event *) object;
    if (com!=0)
    {
        switch(event)
        {
            case  SERIAL_CONNECTED  :
                                        logtx(" Connected ! \n");
                                        threadsWork =TRUE;
                                        g_timeout_add(500,orderInterpreter,gtk_text_view_1);
                                        gtk_widget_set_sensitive(gtk_button_connection,FALSE);
                                        gtk_widget_set_sensitive(gtk_button_execute,TRUE);
                                        break;
            case  SERIAL_DISCONNECTED  :
                                        logtx("Disonnected ! \n");
                                        threadsWork = FALSE;
                                        gtk_widget_set_sensitive(gtk_button_connection,TRUE);
                                        gtk_widget_set_sensitive(gtk_button_execute,FALSE);
                                        break;
            case  SERIAL_DATA_SENT  :
                                        logtx("Data sent ! \n");
                                        break;
            case  SERIAL_RING       :
                                        logtx("DRING ! \n");
                                        break;
            case  SERIAL_CD_ON      :
                                        logtx("Carrier Detected ! \n");
                                        break;
            case  SERIAL_CD_OFF     :
                                        logtx("No more carrier ! \n");
                                        break;
            case  SERIAL_DATA_ARRIVAL  :
                                        logtx("SERIAL_DATA_ARRIVAL\n");
                                        size   = com->getDataInSize();
                                        buffer = com->getDataInBuffer();
                                        onDataArrival(size, buffer);
                                        com->dataHasBeenRead();
                                        break;
        }
    }
}
std::string getCurrentTimestamp()
{
    auto currentTime = std::chrono::system_clock::now();
    auto currentTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime.time_since_epoch()
    );
    std::time_t tt = std::chrono::system_clock::to_time_t(currentTime);
    std::tm timeInfo;
    localtime_s(&timeInfo, &tt);

    std::stringstream ss;
    ss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    ss << ":" << std::setfill('0') << std::setw(3) << currentTimeMs.count() % 1000;

    return ss.str();
}

// ========================================================================================================================

void logtx(std::string str)
{
    std::lock_guard<std::mutex> guard(mutexLogtx);
    logtxVector.push_back(getCurrentTimestamp()+' '+str);
}


gboolean logtxgtk (gpointer data) {

    std::lock_guard<std::mutex> guard(mutexLogtx);
    std::string str;
    for(auto& i:logtxVector)
        str+=i;
    logtxVector.clear();

    GtkTextBuffer *buffer;
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW(gtk_text_view_1) );

    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
    gtk_text_buffer_insert (buffer, &iter, std::string(str).c_str(), -1);
    return TRUE;
}

// ========================================================================================================================


extern "C" G_MODULE_EXPORT void on_window_main_destroy () {

    // zakonczenie polaczenia i skasowanie obiektu
    if(com)
    {
        com->disconnect();
        delete com;
        com = 0;
    }

    gtk_main_quit ();
}
void orderReceivedBuilder()
{
    std::lock_guard<std::mutex> guard(mutexDataIn);
    std::string temp;
    for(int i=0;i<dataIn.size();i++)
        temp += dataIn[i];

    //clear dataIn
    dataIn.clear();
    parseString(temp);
    dataIn.push_back(temp);
}
//===
void parseString(std::string& stringToParse)
{
    size_t delimiterIndex = 0;
    char delimiter = 13;
    size_t i=0;
    while((delimiterIndex = stringToParse.find(delimiter)) != std::string::npos)
    {
        std::lock_guard<std::mutex> guard(mutextimeQueue);

        std::string order = stringToParse.substr(0,delimiterIndex-1);
        receivedOrders.push(std::make_pair(order, timeQueue.front()));
        timeQueue.pop();
        stringToParse.erase(0, delimiterIndex+1);
    }
}

// ========================================================================================================================

void on_gtk_button_execute_clicked (GtkButton *button, gpointer user_data)
{
//    if(!threadsWork)
//    {
//        threadsWork= TRUE;
//        g_timeout_add(1000,orderInterpreter,gtk_text_view_1);
//        g_timeout_add(1000,checkOrders,gtk_text_view_1);
//    }
//    else{
//        threadsWork=FALSE;
//    }
    gtk_widget_set_sensitive(gtk_button_execute,FALSE);
    if(ordersToSend.empty())
    {

        int n = std::atoi(std::string (gtk_entry_get_text(GTK_ENTRY(gtk_entry_1))).c_str());
        if(!n) n=100;
        std::lock_guard<std::mutex> guard(mutexOrdersToSend);
        ordersToSend.push("LED1\n\r");
       // ordersToSend.push("LED1\n\r");
//        ordersToSend.push("NBS\n\r");
//        gtk_label_set_text(GTK_LABEL(gtk_label_1), (std::string("Ilosc RD "+std::to_string(n))).c_str());
//        while(n--)
//        {
//            ordersToSend.push("RD1\n\r");
//        }
        //ordersToSend.push("LED0\n\r");
        gtk_entry_set_text(GTK_ENTRY(gtk_entry_1),"");
    }
        g_timeout_add(200,checkOrders,gtk_text_view_1);
}

void on_gtk_button_saveLogs_clicked(GtkButton* button, gpointer data)
{
    std::fstream file;
    std::string name = "Logs " + getCurrentTimestamp().substr(0,19) + ".txt";
    std::replace(name.begin(),name.end(),':','-');
//    gtk_widget_set_sensitive(gtk_button_saveLogs, FALSE);
    file.open(name,std::ios::out);
    if(file.good())
    {
        GtkTextIter startIter;
        GtkTextIter endIter;
        GtkTextBuffer *buffer=gtk_text_view_get_buffer ( GTK_TEXT_VIEW(gtk_text_view_1));
        gtk_text_buffer_get_start_iter(buffer,&startIter);
        gtk_text_buffer_get_end_iter (buffer, &endIter);
        file <<gtk_text_buffer_get_text(buffer,&startIter,&endIter,-1 );
        file.close();
    }
    else
    {
        logtx("There was a problem opening file " + name+ '\n');
    }
//    gtk_widget_set_sensitive(gtk_button_saveLogs, TRUE);

}

void on_gtk_button_connect_clicked(GtkButton* button, gpointer data)
{
    std::string comName = std::string (gtk_entry_get_text(GTK_ENTRY(gtk_entry_2))).c_str();
    //fix come higher than 9
    if (com!=0)
    {
        com->setManager(serialEventManager);
        error = com->connect(const_cast<char*>(comName.c_str()), 9600, SERIAL_PARITY_NONE, 8, true);
        if (!error)
        {
            com->setRxSize(1);
            logtx("Name: "+comName+"\n");

        }
        else
            logtx("ERROR : com->connect (" + std::to_string(error) + ") Name: "+comName+"\n");

    }
}

gboolean orderInterpreter(gpointer data)
{
    std::lock_guard<std::mutex> guard(mutexReceivedOrders);
    //logtx("jestem w orderInterpreter, sprawdzam if\n");
    if(!receivedOrders.empty())
    {
        std::string receivedOrderData = receivedOrders.front().first;
        std::string receivedOrderTime = receivedOrders.front().second;

        receivedOrders.pop();
        if(receivedOrderData[1]=='.')
        {
            //handle status
            logtx(receivedOrderTime + ' ' + receivedOrderData);

        }
        else
        {//odczyt z komendy
            logtx(receivedOrderTime + ' ' + receivedOrderData);
            std::lock_guard<std::mutex> guard2(mutexFirstCommand);
            firstCommand.clear();
        }
    }
    else{
        //logtx("jestem w orderInterpreter, else chce zbudowac ordery\n");
        orderReceivedBuilder();
    }
return threadsWork;

}

gboolean checkOrders(gpointer data) //wywolywac co jakis czas zeby wysylac ordery
{
    std::lock_guard<std::mutex> guard2(mutexFirstCommand);
    std::lock_guard<std::mutex> guard(mutexOrdersToSend);
    //logtx("jestem w chceckOrders, sprawdzam if\n");
    if(firstCommand.empty() && !ordersToSend.empty())
    {
        //logtx("jestem w chceckOrders, if, number of orders to send:"+ std::to_string(ordersToSend.size()) +"\n");
        firstCommand = ordersToSend.front();
        logtx("firstCommand: " + firstCommand);
        sent=false;
        //logtx("jestem w chceckOrders w if: first command= "+firstCommand+"\n");
        ordersToSend.pop();
    }
    //if(!ordersToSend.empty()) logtx("jestem w chceckOrders number of orders to send:"+ std::to_string(ordersToSend.size()) +"\n");
    if(!sent)
    {
        //logtx("jestem w chceckOrders, sprawdzam if sent, nie wyslalo wiec wysylam\n");
        sendCommand();
    }
    if(firstCommand.empty() && ordersToSend.empty())
    {
        gtk_widget_set_sensitive(gtk_button_execute,TRUE);
        return FALSE;
    }
    return threadsWork;
}

void sendCommand()
{
    char* buf = const_cast<char*>(firstCommand.c_str());
    com->sendData(buf,sizeof(buf));
    sent=true;
}

// ========================================================================================================================

int main( int argc, char *argv[]) {

    gtk_init(&argc, &argv);

    // wczytamy widgety z pliku *.glade za pomoca GtkBuilder
    gbuilder = gtk_builder_new();
    if ( !gtk_builder_add_from_file(gbuilder, "pwi_serial_nasze.glade", &gerror) ) { // jesli wystapill blad
        // klasa diagnostyczna powinna zapisac do pliku: "GBuilder error: " + std::string(gerror->message)
        g_free( gerror );
        return( 1 ); // wyjscie z programu z kodem informacji o bledzie
    }

    gtk_window_1 = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_window_1" ) );
    gtk_button_execute = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_button_execute" ) );
    gtk_button_connection = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_button_connection" ) );
    gtk_button_saveLogs = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_button_saveLogs" ) );
    gtk_entry_1 = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_entry_1" ) );
    gtk_entry_2 = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_entry_2" ) );
    gtk_label_1 = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_label_1" ) );
    gtk_text_view_1 = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_text_view_1" ) );

    gtk_builder_connect_signals( gbuilder, NULL );  // w tym projekcie automatyczne podpiecie tylko on_window_main_destroy()
    g_object_unref ( G_OBJECT( gbuilder ) );     // mozna juz usunac gbuilder

    g_signal_connect ( gtk_button_execute, "clicked", G_CALLBACK(on_gtk_button_execute_clicked), NULL );   // reczne podpiecie funkcji do sygnalu 'clicked' dla widgetu gtk_button_execute
    gtk_widget_set_sensitive(gtk_button_execute,FALSE);

    g_signal_connect ( gtk_button_connection, "clicked", G_CALLBACK(on_gtk_button_connect_clicked), NULL );   // reczne podpiecie funkcji do sygnalu 'clicked' dla widgetu gtk_button_execute

    g_signal_connect ( gtk_button_saveLogs, "clicked", G_CALLBACK(on_gtk_button_saveLogs_clicked), NULL );   // reczne podpiecie funkcji do sygnalu 'clicked' dla widgetu gtk_button_execute

    gtk_window_set_default_size ( GTK_WINDOW(gtk_window_1), 800, 650 );
    gtk_window_set_position ( GTK_WINDOW(gtk_window_1), GTK_WIN_POS_CENTER );
    gtk_widget_show ( gtk_window_1 );

// ========================================================================================================================

    // utworzenie obiektu Tserial_event i proba inicjacji polaczenia RS


    com = new Tserial_event();


// ========================================================================================================================
    g_idle_add(logtxgtk,gtk_text_view_1);
    gtk_main(); // w tej petli program okienkowy tkwi, dop�ki go nie zakonczymy przyciskiem 'X'




// ========================================================================================================================

    // zakonczenie przeniesiono do on_window_main_destroy

    return 0;
}
