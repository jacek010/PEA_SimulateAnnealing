// BranchAndBound.cpp : Ten plik zawiera funkcj� �main�. W nim rozpoczyna si� i ko�czy wykonywanie programu.
//

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <queue>
#include <ctime>
#include <time.h>
#include <stdlib.h>

using namespace std;
#define instancy 16
#define erasAndTimeConfigs 3

int cnt;//ilo�� wierzcho�k�w w grafie
string fileNames[instancy];    //tabela zawieraj�ca nazwy plik�w z konkretnymi instancjami
int instancyCounter[instancy]; //tabela zawieraj�ca informacje ile razy mamy przetestowa� dan� instancj�
int optimumInstancyCost[instancy]; //tabela zawieraj�ca informacje o optymalnej �cie�ce dla danej instancji
int eraLength[erasAndTimeConfigs]; //długość każdej epoki
int neigbourChangeStyle = 0;//1 - 2zamiana || 0-wymiana łuków || 2 - 3zamiana
int coolingStyle = 0;// 0-chłodzenie geometryczne || 1-chłodzenie Boltzmanna || 2-Cauchyego
int generatingTemperatureStyle = 0;//0-na podstawie średniego kosztu permutacji ścieżki || 1-na podstawie ilości wierzchołków i najdłuższego połączenia
int measuringMode = 1; //0 - do określonego czasu 1 - do określonego poziomu błędu
int currentInstancy = 0;
float mistakePercents = 20;
int highestAcceptableCost;
int maxTime = 180;
int erasOutDivider = 3;


ofstream plikWynikowy("daneWynikowe.txt");

vector < vector<int> > miasta; //wektor zawieraj�cy list� wektor�w, w kt�rej indeksem s� numery miast, a warto�ciami odleg�o�ci
int longestRoute=0;
int minCost;

vector <int> shortestPath;
vector <int> tempPath;
double alphaCoolRate = 0.95;
int coolingB = 3;






void loadConfig() //funkcja wczytuje konfiguracj� z pliku tekstowego
{

    ifstream plik;
    plik.open("./dataFiles/config.txt");
    if (!plik.good())
    {
        cout << "Nie udalo sie wczytac pliku, sprobuj ponownie";
        return;
    }
    cout << "Ladowanie konfiguracji..." << endl;
    plik >> measuringMode;
    plik >> mistakePercents;
    plik >> maxTime;
    plik >> alphaCoolRate;
    string name;
    for (int i = 0; i < instancy; i++)
    {
        plik >> name;
        fileNames[i] = "./dataFiles/" + name;
        plik >> instancyCounter[i];
        plik >> optimumInstancyCost[i];
    }

    for (int i = 0; i < erasAndTimeConfigs; i++)
    {
        plik >> eraLength[i];
    }


    plik.close();
}
void loadFromFile(int index) //funkcja wczytuj�ca dane z pliku
{
    longestRoute = 0;
    ifstream plik;
    plik.open(fileNames[index]);
    if (!plik.good())
    {
        cout << "Nie udalo sie wczytac pliku, sprobuj ponownie";
        return;
    }
    cout << "Ladowanie danych..." << endl;
    plik >> cnt;
    cout << "Ilosc miast: " << cnt << endl;
    int li;
    minCost = INT_MAX;
    miasta.clear();
    tempPath.clear();
    shortestPath.clear();
    for (int i = 0; i < cnt; i++)
    {
        vector <int> odleglosc;
        for (int k = 0; k < cnt; k++)
        {
            plik >> li;
            odleglosc.push_back(li);
            if (li > longestRoute) longestRoute = li;
        }
        miasta.push_back(odleglosc);
    }
    plik.close();
    highestAcceptableCost = (1 + mistakePercents / 100) * optimumInstancyCost[index];
    cout << "Wczytywanie zakonczone powodzeniem" << endl;

}

void testData(int era, int time) //funkcja sprawdzaj�ca poprawno�� wczytania danych
{
    cout << endl;
    for (int i = 0; i < cnt; i++)
    {
        for (int k = 0; k < cnt; k++)
        {
            cout << miasta[i][k] << "  ";
        }
        cout << endl;
    }
    cout << "Dlugosc ery: " << eraLength[era] * cnt << endl;

    /*cout <<"\n Wszystkie polaczenia: "<< endl;
    for (int i = 0; i < cnt; i++)
    {
        for (int k = 0; k < cnt; k++)
        {
            cout << "Miasto: "<<i<<" do miasto "<<k<<"  -  odleglosc: "<<miasta[i][k] << endl;
        }
        cout << endl;
    }*/
}

void printShortestPath(int index, int era)
{
    if (measuringMode == 0)
    {
        plikWynikowy << "Maksymalny czas: " <<maxTime<<" s" << endl;
        cout << "Maksymalny czas: " << maxTime << " s" << endl;
    }
    else
    {
        plikWynikowy << "Maksymalny blad: " << mistakePercents << " %" << endl;
        cout << "Maksymalny blad: " << mistakePercents << " %" << endl;
    }
    plikWynikowy << "PLIK WEJSCIOWY" << "\t" << "CZAS[s]\t" << "DLUGOSC SCIEZKI\t" << "GORSZA OD OPTYMALNEJ\t" << "PROCENTOWO" << endl;
    cout << "PLIK WEJSCIOWY" << "\t" << "CZAS[s]\t" << "DLUGOSC SCIEZKI\t" << "GORSZA OD OPTYMALNEJ\t" << "PROCENTOWO" << endl;

    plikWynikowy << "Alpha cool rate: " << alphaCoolRate << endl;
    plikWynikowy << "Sposob generowania temperatury: " << generatingTemperatureStyle << endl;
    plikWynikowy << "Styl chlodzenia: " << coolingStyle << endl;
    plikWynikowy << "Sposob przegladania sasiedztwa " << neigbourChangeStyle << endl;
    plikWynikowy << "Dlugosc ery: " << eraLength[era] << endl;
    cout << "Alpha cool rate: " << alphaCoolRate << endl;
    cout << "Sposob generowania temperatury: " << generatingTemperatureStyle << endl;
    cout << "Styl chlodzenia: " << coolingStyle << endl;
    cout << "Sposob przegladania sasiedztwa " << neigbourChangeStyle << endl;
    cout << "Dlugosc ery: " << eraLength[era] << endl;
    plikWynikowy << "Najkrotsza sciezka: ";
    cout << "Najkrotsza sciezka: ";
    for (auto itr : shortestPath)
    {
        cout << itr << " ";
        plikWynikowy << itr << " ";
    }
    plikWynikowy << endl << "Dlugosc tej sciezki to: " << minCost << endl;
    cout << endl << "Dlugosc tej sciezki to: " << minCost << endl;
    plikWynikowy << endl << "Jest gorsza (dluzsza) od optymalnej o: " << minCost - optimumInstancyCost[index] <<"   Blad: " <<(((float)minCost - (float)optimumInstancyCost[index]) / (float)optimumInstancyCost[index]) * 100 <<"%"<< endl;
    cout << endl << "Jest gorsza (dluzsza) od optymalnej o: " << minCost - optimumInstancyCost[index] << "   Blad: " << (((float)minCost - (float)optimumInstancyCost[index]) / (float)optimumInstancyCost[index]) * 100 << "%" << endl;

    plikWynikowy << "\n\n\n" << endl;
    cout << "\n\n\n" << endl;
}

void printTempPath()
{
    cout << "\n\n Aktualna sciezka: ";
    for (auto itr : tempPath) cout << itr << " ";
    cout << endl << "Dlugosc tej sciezki to: " << minCost << "  "<<(double)minCost/optimumInstancyCost[currentInstancy]*100<<"%" << endl;

}

double coolTemperature(double oldTemperature, int k)
{
    if (coolingStyle == 0) //schemat geometryczny
    {
        return oldTemperature * pow(alphaCoolRate, k);
    }
    else if(coolingStyle==1)//schemat Boltzmann`a
    {
        return (-1 * oldTemperature) / (alphaCoolRate + (coolingB * log(k)));
    }
    else //schemat Cauchy`ego
    {
        return oldTemperature / (alphaCoolRate + coolingB * k);
    }
}

int calculatePathCost(vector<int> path)
{
    int cost = 0;

    for (int i = 0; i < path.size() - 1; i++) cost += miasta[path[i]][path[i + 1]];

    cost+= miasta[path[path.size()-1]][path[0]];

    return cost;
}

vector<int> generateRandomPath()
{
    vector<int> randomPath;
    for (int i = 0; i < cnt; i++)
    {
        randomPath.push_back(i);
    }

    random_shuffle(randomPath.begin(), randomPath.end());

    return randomPath;
}

double generateStartTemperature(int index)
{
    if (generatingTemperatureStyle == 0)
    {
        int swapA, swapB, temp = 0;
        vector<int> base, swapped;
        int iterations = 100;
        double percents = 0;
        for (int i = 0; i < iterations; i++)
        {
            base = generateRandomPath();
            double cost = calculatePathCost(base);
            percents += ((cost - optimumInstancyCost[index]) / optimumInstancyCost[index]) * 100;

            do
            {
                swapA = rand() % cnt;
                swapB = rand() % cnt;
            } while (swapA == swapB);

            swapped = base;
            swap(swapped[swapA], swapped[swapB]);
            temp += fabs(calculatePathCost(base) - calculatePathCost(swapped));
        }
        cout <<"Srednia losowa sciezka gorsza od optymalnej o: " << percents / iterations <<" %" << endl;
        temp /= iterations;
        return (-1 * temp) / log(alphaCoolRate);
    }
    else return longestRoute*longestRoute * pow(cnt,2);
}

double getProbability(int diff, double temperature)
{
    double proba = exp(diff / temperature);
    return proba;
}

void simulateAnnealing(double startTemperature, int eras)
{
    double time=0;
    double temperature = startTemperature;
    int swapA, swapB,swapC, diff;
    clock_t start=clock();
    //długość każdej epoki - eras
    int erasWithoutChange = 0;
    minCost = INT_MAX;
    int k;
    int temp;
    srand(clock());
    
    

    tempPath=generateRandomPath();
    vector<int> nextTempPath(tempPath);
    //printTempPath();
        while (true)
        {
            srand(clock());
            k = 0;
            int bestCost = INT_MAX;
            //numer epoki
            while (temperature >= 0.1)
            {
                k++;
                erasWithoutChange = 0;
                nextTempPath = tempPath;

                int nextCost = calculatePathCost(nextTempPath);


                for (int e = 0; e < eras; e++)
                {
                    erasWithoutChange++;
                    swapA = rand() % cnt;
                    do
                    {
                        swapB = rand() % cnt;
                    } while (swapA == swapB);

                    if (neigbourChangeStyle == 0)//dwuzamiana
                    {
                        swap(nextTempPath[swapA], nextTempPath[swapB]);
                    }
                    else if (neigbourChangeStyle == 1)//wymiana łuków
                    {
                        if (swapA < swapB)reverse(nextTempPath.begin() + swapA, nextTempPath.begin() + swapB);
                        else reverse(nextTempPath.begin() + swapB, nextTempPath.begin() + swapA);
                    }
                    else
                    {
                        do
                        {
                            swapC = rand() % cnt;
                        } while (swapA == swapC || swapB == swapC);
                        temp = nextTempPath[swapA];
                        nextTempPath[swapA] = nextTempPath[swapB];
                        nextTempPath[swapB] = nextTempPath[swapC];
                        nextTempPath[swapC] = temp;
                    }

                    nextCost = calculatePathCost(nextTempPath);

                    diff = bestCost - nextCost;

                    if (diff > 0)
                    {
                        erasWithoutChange = 0;
                        bestCost = nextCost;
                        
                        tempPath = nextTempPath;
                        //printTempPath();

                    }
                    else if (getProbability(diff, temperature)>((double)rand() / RAND_MAX))
                    {
                        erasWithoutChange = 0;
                        tempPath = nextTempPath;
                        //printTempPath();
                        break;
                    }
                    else
                    {
                        if (neigbourChangeStyle == 0)
                        {
                            swap(nextTempPath[swapA], nextTempPath[swapB]);
                        }
                        else if (neigbourChangeStyle == 1)
                        {
                            if (swapA < swapB)reverse(nextTempPath.begin() + swapA, nextTempPath.begin() + swapB);
                            else reverse(nextTempPath.begin() + swapB, nextTempPath.begin() + swapA);
                        }
                        else
                        {
                            temp = nextTempPath[swapC];
                            nextTempPath[swapC] = nextTempPath[swapA];
                            nextTempPath[swapA] = nextTempPath[swapB];
                            nextTempPath[swapB] = temp;
                        }
                    }
                    if (erasWithoutChange > eras / erasOutDivider) break;
                }
                if (bestCost < minCost)
                {
                    shortestPath = tempPath;
                    minCost = bestCost;
                }
                if (measuringMode == 0)//do określonego czasu i sprawdzamy błąd
                {
                    if (minCost <= highestAcceptableCost) return;
                    time = (clock() - start) / (double)CLOCKS_PER_SEC;
                    if (time >= maxTime)
                    {
                        return;
                    }
                }
                else//do określonego poziomu błędu
                {
                    if (minCost <= highestAcceptableCost) return;
                }
                temperature = coolTemperature(temperature, k);
            }
            
            
            temperature = startTemperature;
            tempPath = generateRandomPath();

        }
        
}



int main()
{
    LARGE_INTEGER StartingTime, EndingTime;
    LARGE_INTEGER DifferenceOfTime = { 0 };
    LARGE_INTEGER frequency; QueryPerformanceFrequency(&frequency);

    loadConfig();   //funkcja wczytuje konfiguracj� programu 

    //testData();     //funkcja wy�wietla na ekranie wczytane dane, aby mo�na by�o sprawdzi� ich poprawno��

   
    
        
    
        for (generatingTemperatureStyle = 0; generatingTemperatureStyle < 2; generatingTemperatureStyle++)
        {
            for (neigbourChangeStyle = 0; neigbourChangeStyle < 3; neigbourChangeStyle++)
            {

            for (coolingStyle = 0; coolingStyle < 3; coolingStyle++)
            {
                for (int e = 0; e < erasAndTimeConfigs; e++)
                {
                    for (int k = 0; k < instancy; k++)
                    {

                        currentInstancy = k;


                        //plikWynikowy << fileNames[k]<<endl;
                        loadFromFile(currentInstancy); //funkcja wczytuje dane z pliku
                        //testData(e, t);
                        for (int i = 0; i < instancyCounter[currentInstancy]; i++)
                        {
                            QueryPerformanceCounter(&StartingTime);
                            simulateAnnealing(generateStartTemperature(currentInstancy), pow(cnt, eraLength[e]));   //funkcja właściwa dla symulowanego wyżarzania
                            QueryPerformanceCounter(&EndingTime);

                            DifferenceOfTime.QuadPart = (EndingTime.QuadPart - StartingTime.QuadPart) * 1000000000 / frequency.QuadPart; //w nanosekundach

                            plikWynikowy << fileNames[currentInstancy] << "\t" << DifferenceOfTime.QuadPart << "ns\t" << minCost << "\t" << minCost - optimumInstancyCost[currentInstancy] << "\t" << (((float)minCost - (float)optimumInstancyCost[currentInstancy]) / (float)optimumInstancyCost[currentInstancy]) * 100 << "%" << endl;
                            cout << fileNames[currentInstancy] << "\t" << DifferenceOfTime.QuadPart << "ns\t" << minCost << "\t" << minCost - optimumInstancyCost[currentInstancy] << "\t" << (((float)minCost - (float)optimumInstancyCost[currentInstancy]) / (float)optimumInstancyCost[currentInstancy]) * 100 << "%" << endl;
                        }

                        printShortestPath(currentInstancy, e); //funkcja wypisuje znalezion� najkr�tsz� �cie�k� wraz z jej d�ugo�ci�
                    }
                }
            }
        }
    }

    //printShortestPath(); //funkcja wypisuje znalezion� najkr�tsz� �cie�k� wraz z jej d�ugo�ci�
    plikWynikowy.close();

    getchar(); //blokada przez samoczynnym zamkni�ciem si� okna
}