
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
    GtkWidget   *gtk_button_1;
    GtkWidget   *gtk_entry_1;
    GtkWidget   *gtk_label_1;
    GtkWidget   *gtk_text_view_1;

    Tserial_event *com;
    std::mutex mutexDataIn;
    std::vector<std::string> data_in;
    std::mutex mutexReceivedOrders;
    std::queue<std::pair<std::string,std::string>> receivedOrders; //struct w ktorym bedzie time string
    std::queue<std::string> timeVector;
    std::mutex mutexTimeVector;
    std::mutex mutexOrdersToSend;
    std::mutex mutexFirstCommand;
    std::queue<std::string> ordersToSend;
    std::string firstCommand;
    std::thread *sendThread;
    std::thread *interpretThread;


    bool sent= false;

// ========================================================================================================================

void logtx (std::string str);
void parseString(std::string& stringToParse);
void send_command();
std::string getCurrentTimestamp();
void INTERPRET_THREAD();
void SEND_THREAD();
void checkOrders();
void orderInterpreter();

// ========================================================================================================================

/* ======================================================== */
/* ===============  OnCharArrival     ===================== */
/* ======================================================== */
void OnDataArrival(int size, char *buffer)
{
    std::lock_guard<std::mutex> guard(mutexDataIn);
    if ((size>0) && (buffer!=0))
    {

        buffer[size] = 0;
//        logtx("OnDataArrival: " + std::string(buffer));
        gtk_label_set_text(GTK_LABEL(gtk_label_1), buffer);
        data_in.push_back(std::string(buffer));
        if(*buffer=='\r')
        {
            std::lock_guard<std::mutex> guard2(mutexTimeVector);
            timeVector.push("time"/*getCurrentTimestamp()*/);
        }

    }

    //std::vector<std::string> orders_tmp = split(data_in, '\n');
    //orders.insert(orders.end(), orders_tmp.begin(), orders_tmp.end())
}

/* ======================================================== */
/* ===============  OnCharArrival     ===================== */
/* ======================================================== */
void SerialEventManager(void* object, uint32 event)    // BYLO: void SerialEventManager(uint32 object, uint32 event)
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
                                        logtx("Connected ! \n");

                                       sendThread->join();
                                       interpretThread->join();
                                        break;
            case  SERIAL_DISCONNECTED  :
                                        logtx("Disonnected ! \n");
                                        sendThread->detach();
                                        interpretThread->detach();
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
                                        OnDataArrival(size, buffer);
                                        com->dataHasBeenRead();
                                        break;
        }
    }
}
std::string getCurrentTimestamp()
{
    using std::chrono::system_clock;
    auto currentTime = std::chrono::system_clock::now();
    char buffer[80];

    auto transformed = currentTime.time_since_epoch().count() / 1000000;

    auto millis = transformed % 1000;

    std::time_t tt;
    tt = system_clock::to_time_t(currentTime);
    auto timeinfo = localtime(&tt);
    strftime(buffer, 80, "%F %H:%M:%S", timeinfo);
    sprintf(buffer, "%s:%03d", buffer, (int)millis);

    return std::string(buffer);
}

// ========================================================================================================================

void logtx (std::string str) {

    GtkTextBuffer *buffer;
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW(gtk_text_view_1) );

    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
    gtk_text_buffer_insert (buffer, &iter, std::string(str + "\n").c_str(), -1);
}

// ========================================================================================================================


extern "C" G_MODULE_EXPORT void on_window_main_destroy () {

    // zakonczenie polaczenia i skasowanie obiektu
    com->disconnect();
    delete com;
    com = 0;

    //deleting threads
    delete sendThread ;
    delete interpretThread ;
    sendThread =0;
    interpretThread =0;

    gtk_main_quit ();
}
//
void orderReceivedBuilder()
{
    std::lock_guard<std::mutex> guard(mutexDataIn);
    //std::lock_guard<std::mutex> guardQueue(mutexReceivedOrders); ///jesli cos nie dziala to przenies mutex do parseString
    std::string temp = "";
    for(int i=0;i<data_in.size();i++)
        temp += data_in[i];

    //clear data_in
    data_in.clear();
    parseString(temp);
    data_in.push_back(temp);
}
//===
void parseString(std::string& stringToParse)
{
    size_t delimiterIndex = 0;
    char delimiter = 13;
    size_t i=0;
    while((delimiterIndex = stringToParse.find(delimiter)) != std::string::npos)
    {
        std::lock_guard<std::mutex> guard(mutexTimeVector);

        std::string order = stringToParse.substr(0,delimiterIndex-1);
        receivedOrders.push(std::make_pair(order, timeVector.front()));
        timeVector.pop();
        stringToParse.erase(0, delimiterIndex+1);
    }
}
//bool checkCommand(const std::string& com)
//{
//    std::vector<std::string> v = {"STA", "RD", "LED", "NBS"};
//    ///po jednej literce
//
//    return (std::find(v.begin(), v.end(), com) != v.end()) ? true : false;
//}

// ========================================================================================================================

void on_gtk_button_1_clicked (GtkButton *button, gpointer user_data) {
    //com->sendData("Hello World\n",sizeof("Hello World\n"));
//    logtx ("len: " + std::to_string(data_in.size()));a
//    for (unsigned int idx = 0; idx < data_in.size(); idx++)
//        logtx(data_in[idx]);

//    std::lock_guard<std::mutex> guard(mutexDataIn);
//    data_in.clear();
    //std::string command = std::string (gtk_entry_get_text(GTK_ENTRY(gtk_entry_1))); ///pobierz komende z pola tekstowego
    std::lock_guard<std::mutex> guard(mutexOrdersToSend);
    if(ordersToSend.empty())//checkCommand(command)) ///sprawdz czy komenda jest poprawna
    {
        int n=100;

        //ordersToSend.push("STA\n\r");
        ordersToSend.push("LED0\n\r");
        ordersToSend.push("LED1\n\r");

//        ordersToSend.push("NBS\n\r");
        while(n--)
        {
            ordersToSend.push("RD1\n\r");
        }
        ordersToSend.push("LED0\n\r");
        gtk_entry_set_text(GTK_ENTRY(gtk_entry_1),"");
    }
    // int n=100;
    // while(n--)
    // {
    //     checkOrders();
    //     orderInterpreter();
    //     Sleep(200);
    // }
}

void orderInterpreter()
{
    std::lock_guard<std::mutex> guard(mutexReceivedOrders);
    logtx("jestem w orderInterpreter, sprawdzam if\n" /*+  getCurrentTimestamp() +"\n"*/);
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
        else{
            //odczyt z komendy
            logtx(receivedOrderTime + ' ' + receivedOrderData);
        }
        std::lock_guard<std::mutex> guard2(mutexFirstCommand);
        firstCommand.clear();
        sent = false;
        }
    else{
        logtx("jestem w orderInterpreter, else chce zbudowac ordery\n");
        orderReceivedBuilder();
    }


}

void checkOrders() //wywolywac co jakis czas zeby wysylac ordery
{
    std::lock_guard<std::mutex> guard2(mutexFirstCommand);
    std::lock_guard<std::mutex> guard(mutexOrdersToSend);
    logtx("jestem w chceckOrders, sprawdzam if\n" /*+  getCurrentTimestamp() +"\n"*/);
    if(firstCommand.empty() && !ordersToSend.empty())
    {
        logtx("jestem w chceckOrders, if, number of orders to send:"+ std::to_string(ordersToSend.size()) +"\n");
        firstCommand = ordersToSend.front();
//        sent=false;
        logtx("jestem w chceckOrders w if: first command= "+firstCommand+"\n");
        ordersToSend.pop();
    }
    if(!ordersToSend.empty()) logtx("jestem w chceckOrders number of orders to send:"+ std::to_string(ordersToSend.size()) +"\n");
    if(!sent)
    {
        logtx("jestem w chceckOrders, sprawdzam if sent, nie wyslalo wiec wysylam\n");
        send_command();
    }
}

void send_command()
{
    //std::lock_guard<std::mutex> guard(mutexFirstCommand);
    char* buf = const_cast<char*>(firstCommand.c_str());
    com->sendData(buf,sizeof(buf));
    sent=true;
}

void SEND_THREAD()
{
    while(1)
    {
        checkOrders();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
}

void INTERPRET_THREAD()
{
    while(1)
    {
        orderInterpreter();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
}

// ========================================================================================================================

int main( int argc, char *argv[]) {

    gtk_init(&argc, &argv);

    // wczytamy widgety z pliku *.glade za pomoca GtkBuilder
    gbuilder = gtk_builder_new();
    if ( !gtk_builder_add_from_file(gbuilder, "pwi_serial.glade", &gerror) ) { // jesli wystapill blad
        // klasa diagnostyczna powinna zapisac do pliku: "GBuilder error: " + std::string(gerror->message)
        g_free( gerror );
        return( 1 ); // wyjscie z programu z kodem informacji o bledzie
    }

    gtk_window_1 = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_window_1" ) );
    gtk_button_1 = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_button_1" ) );
    gtk_entry_1 = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_entry_1" ) );
    gtk_label_1 = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_label_1" ) );
    gtk_text_view_1 = GTK_WIDGET ( gtk_builder_get_object( gbuilder, "gtk_text_view_1" ) );

    gtk_builder_connect_signals( gbuilder, NULL );  // w tym projekcie automatyczne podpiecie tylko on_window_main_destroy()
    g_object_unref ( G_OBJECT( gbuilder ) );     // mozna juz usunac gbuilder

    g_signal_connect ( gtk_button_1, "clicked", G_CALLBACK(on_gtk_button_1_clicked), NULL );   // reczne podpiecie funkcji do sygnalu 'clicked' dla widgetu gtk_button_1

    gtk_window_set_default_size ( GTK_WINDOW(gtk_window_1), 800, 650 );
    gtk_window_set_position ( GTK_WINDOW(gtk_window_1), GTK_WIN_POS_CENTER );
    gtk_widget_show ( gtk_window_1 );

// ========================================================================================================================

    // utworzenie obiektu Tserial_event i proba inicjacji polaczenia RS
    int error = 0;

    com = new Tserial_event();
    if (com!=0)
    {
        com->setManager(SerialEventManager);
        error = com->connect("COM5", 115200, SERIAL_PARITY_NONE, 8, true);
        if (!error)
        {
            //com->sendData("Hello World\n");
            com->setRxSize(1);

           sendThread = new std::thread (SEND_THREAD);
           interpretThread = new std::thread(INTERPRET_THREAD);
        }
        else
            logtx("ERROR : com->connect (" + std::to_string(error) + ")\n");

    }

// ========================================================================================================================

    gtk_main(); // w tej petli program okienkowy tkwi, dop�ki go nie zakonczymy przyciskiem 'X'




// ========================================================================================================================

    // zakonczenie przeniesiono do on_window_main_destroy

    return 0;
}
