#include <iostream>

#include <vector>

#include <thread>

#include <mutex>

#include <condition_variable>

#include <semaphore.h>

#include <chrono>

#include <fstream>

#include <sstream>

#include <ctime>

#include <iomanip>

#include <queue>

#include <cstring>

#include <cerrno>

#include <iomanip>

using namespace std;


int SIMULATION_DURATION = 120;
int BURGER_COOK_TIME = 5;
int FRIES_COOK_TIME = 3;
int COKE_COOK_TIME = 1;
int EATING_TIME = 30;

class Restaurant {
  public: Restaurant(int diners, int numTables, int numCooks, vector < vector < int > > & orders);
  void runSimulator();

  private: int numTables;
  int numCooks;
  int dinerCount;

  vector < vector < int > > dinerOrders;

  vector < thread > dinerThreads;
  vector < thread > cookThreads;
  vector < sem_t > machineSemaphores;
  queue < vector < int > > dinerOrderQueue;
  sem_t orderQueueSemaphore;
  sem_t totalNumDiners;
  sem_t printTimeStampSemaphore;

  vector < mutex > dinerOrderMutexArray;
  vector < condition_variable > dinerOrderCvArray;
  vector < bool > dinerOrderCookedArray;

  sem_t availableTable;
  queue < int > availableTablesQueue;

  void dinerThread(int dinerId);
  void cookThread(int cookId);
  int waitForTable(int dinerId);
  int waitForCook(int dinerId);
  void placeOrder(int dinerId, int burger, int fries, int coke);
  bool isdinerOrderCookedArray(int dinerId);
  void printCurrentTimestamp();
  void currentTimestamp();
};

vector < mutex > initializeMutexVector(int numMutexes) {
  return vector < mutex > (numMutexes);
}

vector < condition_variable > initializeCvVector(int numMutexes) {
  return vector < condition_variable > (numMutexes);
}

Restaurant::Restaurant(int diners, int tables, int cooks, vector < vector < int > > & orders) {

  numTables = tables;
  numCooks = cooks;
  dinerOrders = orders;

  dinerCount = diners;
  sem_init( & printTimeStampSemaphore, 1, 1);
  sem_init( & availableTable, 0, numTables);
  sem_init( & orderQueueSemaphore, 0, 1);
  sem_init( & totalNumDiners, 0, 1);

  queue < int > q;
  for (int i = 0; i < numTables; i++) {
    q.push(i);
  }

  availableTablesQueue = q;

  dinerThreads.reserve(numTables);
  cookThreads.reserve(numCooks);
  machineSemaphores.resize(3);

  vector < bool > orderCookedArray(diners, false);

  dinerOrderMutexArray = initializeMutexVector(diners);
  dinerOrderCvArray = initializeCvVector(diners);
  dinerOrderCookedArray = orderCookedArray;

  for (int i = 0; i < 3; ++i) {
    sem_init( & machineSemaphores[i], 0, 1);
  }
}

void Restaurant::runSimulator() {
  printCurrentTimestamp();
  sem_post( & printTimeStampSemaphore);

  for (int i = 0; i < numCooks; i++) {
    cookThreads.emplace_back( & Restaurant::cookThread, this, i);
  }
  

  for (int i = 0; i < dinerOrders.size(); i++) {
    dinerThreads.emplace_back( & Restaurant::dinerThread, this, i);
  }

  for (int i = 0; i < dinerThreads.size(); i++) {
    dinerThreads[i].join();
  }
  for (int i = 0; i < cookThreads.size(); i++) {
    cookThreads[i].join();
  }

}
bool Restaurant::isdinerOrderCookedArray(int dinerId) {
  return dinerOrderCookedArray[dinerId];
}

int Restaurant::waitForTable(int dinerId) {
  int tableId = -1;
  sem_wait( & availableTable);
  tableId = availableTablesQueue.front();
  availableTablesQueue.pop();
  printCurrentTimestamp();
  cout << " Diner " << dinerId + 1 << " seated at table " << tableId + 1 << "." << endl;
  sem_post( & printTimeStampSemaphore);
  return tableId;
}

void Restaurant::dinerThread(int dinerId) {

  int arrivalTime = dinerOrders[dinerId][0];
  int burgers = dinerOrders[dinerId][1];
  int fries = dinerOrders[dinerId][2];
  int coke = dinerOrders[dinerId][3];

  this_thread::sleep_for(chrono::seconds(arrivalTime));
  printCurrentTimestamp();
  cout << " Diner " << dinerId + 1 << " arrives" << "." << endl;

  sem_post( & printTimeStampSemaphore);
  int tableId = waitForTable(dinerId);
  placeOrder(dinerId, burgers, fries, coke);
  unique_lock < mutex > lock(dinerOrderMutexArray[dinerId]);

  dinerOrderCvArray[dinerId].wait(lock, [this, dinerId]() {
    return isdinerOrderCookedArray(dinerId);
  });
  printCurrentTimestamp();
  cout << " Diner " << dinerId + 1 << "'s order is ready. Diner " << dinerId + 1 << " starts eating." << endl;
  sem_post( & printTimeStampSemaphore);
  this_thread::sleep_for(chrono::seconds(EATING_TIME));
  availableTablesQueue.push(tableId);
  sem_post( & availableTable);
  printCurrentTimestamp();
  cout << " Diner " << dinerId + 1 << " finishes. Diner " << dinerId + 1 << " leaves the restaurant." << endl;

  sem_post( & printTimeStampSemaphore);

  sem_wait( & totalNumDiners);
  dinerCount -= 1;
  if (dinerCount == 0) {
    printCurrentTimestamp();
    cout << "The last diner leaves the restaurant." << endl;

    sem_post( & printTimeStampSemaphore);
    exit(0);
  }
  sem_post( & totalNumDiners);
}

void Restaurant::cookThread(int cookId) {
  while (true) {
    vector < int > order;
    while (order.size() == 0) {
      sem_wait( & orderQueueSemaphore);
      if (dinerOrderQueue.size() > 0) {
        order = dinerOrderQueue.front();
        dinerOrderQueue.pop();
        printCurrentTimestamp();
        cout << " Cook " << cookId + 1 << " processes Diner " << order[0] + 1 << "'s order." << endl;
        sem_post( & printTimeStampSemaphore);
      }
      sem_post( & orderQueueSemaphore);
    }

    if (order[1] > 0) {
      sem_wait( & machineSemaphores[0]);
      printCurrentTimestamp();
      cout << " Cook " << cookId + 1 << " uses burger machine." << endl;
      sem_post( & printTimeStampSemaphore);
      this_thread::sleep_for(chrono::seconds(order[1] * BURGER_COOK_TIME));
      sem_post( & machineSemaphores[0]);
    }

    if (order[2] > 0) {
      sem_wait( & machineSemaphores[1]);
      printCurrentTimestamp();
      cout << " Cook " << cookId + 1 << " uses fries machine." << endl;
      sem_post( & printTimeStampSemaphore);
      this_thread::sleep_for(chrono::seconds(order[2] * FRIES_COOK_TIME));
      sem_post( & machineSemaphores[1]);
    }

    if (order[3] > 0) {
      sem_wait( & machineSemaphores[2]);
      printCurrentTimestamp();
      cout << " Cook " << cookId + 1 << " uses coke machine." << endl;

      sem_post( & printTimeStampSemaphore);
      this_thread::sleep_for(chrono::seconds(order[3] * COKE_COOK_TIME));
      sem_post( & machineSemaphores[2]);
    } {
      unique_lock < mutex > lock(dinerOrderMutexArray[order[0]]);
      dinerOrderCookedArray[order[0]] = true;
    }
    dinerOrderCvArray[order[0]].notify_one();
  }
}

void Restaurant::placeOrder(int dinerId, int burger, int fries, int coke) {
  sem_wait( & orderQueueSemaphore);
  vector < int > k = {
    dinerId,
    burger,
    fries,
    coke
  };
  dinerOrderQueue.push(k);
  sem_post( & orderQueueSemaphore);
}
void Restaurant::printCurrentTimestamp() {
  sem_wait( & printTimeStampSemaphore);
  currentTimestamp();
}
void Restaurant::currentTimestamp() {
  static chrono::time_point < chrono::high_resolution_clock > startTime;

  if (startTime.time_since_epoch().count() == 0) {
    startTime = chrono::high_resolution_clock::now();
  } else {
    auto currentTime = chrono::high_resolution_clock::now();
    auto elapsedTime = chrono::duration_cast < chrono::seconds > (currentTime - startTime);
    int minutes, hours = 0;

    if (elapsedTime.count() >= 60) {
      hours = elapsedTime.count() / 60;
    }
    minutes = elapsedTime.count() % 60;
    printf("%02d", hours);
    cout << ":";
    printf("%02d", minutes);
    cout << " -";
  }
}

int main() {
  int numDiners, numTables, numCooks;
  int actualNumDiners = 0;
  ifstream inputFile("input.txt");

  inputFile >> numDiners;
  inputFile >> numTables;
  inputFile >> numCooks;

  if (numDiners <= 0) {
    cout << "Diners cannot be less than 0" << endl;
    exit(-1);
  }
  if (numTables <= 0) {
    cout << "Tables cannot be less than 0" << endl;
    exit(-1);
  }
  if (numCooks <= 0) {
    cout << "Cooks cannot be less than 0" << endl;
    exit(-1);
  }
  inputFile.ignore();
  vector < vector < int > > dinerOrders(numDiners, vector < int > (4));

  string line;
  for (int i = 0; i < numDiners; ++i) {
    getline(inputFile, line);
    istringstream iss(line);
    char delimiter;
    int arrivalTime, burger, fries, coke;
    iss >> dinerOrders[i][0] >> delimiter >> dinerOrders[i][1] >> delimiter >> dinerOrders[i][2] >> delimiter >> dinerOrders[i][3];
    if (dinerOrders[i][0] < 0) {
      cout << "Diner Arrival time cannot be negative" << endl;
      exit(-1);
    }
    if (dinerOrders[i][0] > SIMULATION_DURATION) {
      break;
    } else {
      actualNumDiners += 1;
    }
  }
  dinerOrders.resize(actualNumDiners);
  Restaurant restaurant(actualNumDiners, numTables, numCooks, dinerOrders);
  restaurant.runSimulator();

  return 0;
}