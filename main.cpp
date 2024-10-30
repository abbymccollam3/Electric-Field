//
// Created by Abby McCollam on 9/27/23.
//
/*
Author: Abby McCollam
Class: ECE4122 Section A
Last Date Modified: 9/27/23
Description:

Main function prompting user for number of rows and columns, separation distances, charge value, and point location.

*/

//directives
#include <iostream>
#include <vector>
#include <thread>
#include <iomanip>
#include <chrono>
#include <istream>
#include <cmath>
#include "ECE_ElectricField.h"

using namespace std;

//GLOBAL VARIABLES
double r; //radius
double q; //charge
double tempEx, tempEy, tempEz, Emag; //temp variables to add electric fields to
double x_sep, y_sep; //separation distances
int row, col; //number of rows and columns
vector<ECE_ElectricField> myArray; //array of electric fields

bool checkForNaturalNumber (int a, int b) //checking for valid natural number inputs
{
    if (a >= 1 && b >=1 && !cin.fail()) // cin.fail() makes sure input is same type as definition
    {
        return true; //if input is valid, return true
    }
    else
    {
        cin.clear(); //clears inputs
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); //ignores input buffer
        return false;
    }
}

bool checkForPositiveDouble (double a, double b) //checking for valid inputs for r and q
{
    if (a > 0.0 && b > 0.0) //making sure both parameters are doubles greater than 0.0
    {
        return true; //if input is valid, return true
    }
    else
    {
        cin.clear(); //clears inputs
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); //ignores input buffer
        return false;
    }
}

void howToCreate2DArray(vector<ECE_ElectricField>& array, int n, int m, double v) //creating 2D array centered around the origin with the given parameters
{
    for (int i = 0; i<n; i++)
    {
        for (int j = 0; j<m; j++)
        {
            double begX = -0.5 * (n - 1) * x_sep;
            double begY = 0.5 * (m - 1) * y_sep;
            array.emplace_back(begX + i * x_sep, begY - j * y_sep, 0.0, v);
        }
    }
}

bool CheckForNM() //checking if row and col are natural numbers
{
    HERE: cout << "Please enter the number of rows and columns in the N x M array: ";
    cin >> row >> col; //user enters number rows

    if (checkForNaturalNumber (row, col))
    {
        return true;
    }
    else
    {
        cout << "Incorrect input. Please enter valid dimensions." << endl;
        goto HERE;
    }
}

bool CheckForDist() //checking for valid separation distances
{
    DIST: cout << "Please enter the x and y separation distances in meters: ";
    cin >> x_sep >> y_sep; //user enters separation distances x and y

    if (checkForPositiveDouble(x_sep, y_sep))
    {
        return true;
    }
    else
    {
        cout << "Incorrect input. Please enter valid separation distances." << endl;
        goto DIST;
    }
}

bool CheckForCharge() //checking for valid charge input
{
    CHARGE: cout << "Please enter the common charge on the points in micro C: "; //user enters charge
    cin >> q;

    if (!cin.fail())
    {
        q *= .000001;
        return false;
    }
    else
    {
        cout << "Incorrect input. Please enter a valid charge." << endl;
        cin.clear();
        cin.ignore();
        goto CHARGE;
    }
}

bool CheckForLoc(double x_loc, double y_loc, double z_loc) //checking for any numerical input
{
    bool locationMatches = false;

    if (!cin.fail()) //if the input is good
    {
        for (auto &q: myArray) //see if location is same as a point charge
        {
            if (q.getX() == x_loc && q.getY() == y_loc && q.getZ() == z_loc)
            {
                cout << "Location entered is the same as a point charge location." << endl;
                locationMatches = true;
                break;
            }
        }
        if (locationMatches) //if location matches, return false and try again
        {
            return false;
        }
        else //otherwise return true
        {
            return true;
        }
    }
    else //if the input is automatically bad, try again
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a valid location." << endl;
        return false;
    }
}

void SumUp() //need to add up total number of charges
{
    for (const auto& q : myArray)
    {
        double Ex, Ey, Ez;
        q.getElectricField(Ex, Ey, Ez);
        tempEx += Ex;
        tempEy += Ey;
        tempEz += Ez;
    }
}

bool ContinueFunc() //does the user want to continue with the program
{
    cout << "Do you want to continue? (Yes/No) ";

    string cont; //cont variable to signal continue or not
    cin >> cont;

    if (cont == "Yes" || cont == "yes") //checking if the user wants to continue
    {
        return true;
    }
    else
    {
        cout << "Done." << endl;
        return false;
    }
}

void printNow() // printing values of electric field
{
    Emag = sqrt(tempEx * tempEx + tempEy * tempEy + tempEz * tempEz); //calculating total electric field

    cout << "Ex = " << scientific << setprecision(4) << tempEx << endl;
    cout << "Ey = " << scientific << tempEy << endl;
    cout << "Ez = " << scientific << tempEz << endl;
    cout << "|Ez| = " << scientific << Emag << endl;
}

int main()
{
    double x, y, z; //point location

    unsigned int n = thread::hardware_concurrency(); //setting n equal to hardware concurrency
    cout << "This system can execute " << n << " threads." << endl; //notifies user about how many threads are running concurrently

    CheckForNM();
    CheckForDist();
    CheckForCharge();

    howToCreate2DArray(myArray, row, col, q); //creates 2D array using function

    while (true) //while loop the code is circling through that keeps prompting user for new inputs
    {
        LOC: cout << "Please enter the location in space to determine the electric field (x y z) in meters: ";
        cin >> x >> y >> z;

        if (CheckForLoc(x, y, z))
        {
            //return true;
        }
        else
        {
            goto LOC;
        }

        auto start_time = chrono::high_resolution_clock::now(); //start time
        vector<thread> threads;

        for (auto& q : myArray) //computing electric field at given point
        {
            threads.emplace_back([&q, x, y, z]()
             {
                 q.computeFieldAt(x, y, z);
             });
        }

        // Check status and wait for all threads to finish
        for (auto& thread : threads)
        {
            thread.join();
        }

        SumUp();

        auto end_time = chrono::high_resolution_clock::now(); //end time
        auto duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time); //calculate time taken in milliseconds

        cout << "The electric field at (" << floor(x) << ", " << floor(y) << ", " << floor(z) << ") in V/m is" << endl; //outputting electric field using member functions
        printNow();
        cout << "The calculation took " << duration.count() << " microseconds!" << endl; //printing total time

        if (!ContinueFunc())
        {
            break;
        }
        else
        {
            continue;
        }
    }
}