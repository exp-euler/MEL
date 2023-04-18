#include <vector>
#define MEL_IMPLEMENTATION
#include "MEL.hpp"
#include "MEL_deepcopy.hpp"

#include<Eigen/Dense>
#include<iostream>


// TESTING FROM PAPER
struct StructA {
 std::vector<int> arr;
};
// External - Global free deep-copy function
template<typename MSG> void StructA_DeepCopy(StructA &obj, MSG
&msg) {
msg & obj.arr;
}

// External - Global free deep-copy function for vectors of pointers
template<typename MSG> void VecP_DeepCopy(double &obj, MSG &msg) {
    //msg.packPtr(obj);
}

int main(int argc, char *argv[]) {
    MEL::Init(argc,argv);

    MEL::Comm comm = MEL::Comm::Comm::WORLD;
    const int rank = MEL::CommRank(comm);

    // Send a class that has a member MatrixXd
    // Eigen::MatrixXd TEST {{1,2},{3,4}};
    //
    // Send by pointer to the class object as well as by value

    if(rank == 0){
        /*
        Eigen::MatrixXd T0 {{1,2,3},{4,5,6},{7,8,9}};
        Eigen::MatrixXd T1;// {{0,0,0},{0,0,0},{0,0,0}};
        T1.resize(3,3);
        //Eigen::MatrixXd T1(3,3);
        //Eigen::ArrayXd T0(2);
        //T0(0) = 1.0; T0(1) = 2.0;
        std::cout << T0 << "\n";
        for(int i=0; i<3*3; i++){
            *(&(T1(0,0))+ i) = *(&(T0(0,0))+ i);
        }
        std::cout << T1 << "\n";

        std::vector<double> V0 {1.0, 3.5, 2.5};
        std::vector<double> V1;
        V1.resize(V0.size());
        for(int i=0; i<V0.size(); i++){
            *(&(V1[0]) + i) = *(&(V0[0]) + i);
        }
        std::cout << "V0[1]" << V0[1] << "\n";
        std::cout << "V1[1]" << V1[1] << "\n";
        */

        //std::vector<double> T0 {1.0, 3.5, 2.5};
        double d0 = 2.4;
        std::vector<double> T0 {d0};
        std::cout << "Rank0: " << "\n";
        std::cout << T0[0] << "\n";
        MEL::Deep::Send<std::vector<double>, MEL::Deep::PointerHashMap, VecP_DeepCopy> (T0, 1, 99, comm);

        /*
        Eigen::MatrixXd T0 {{1,2,3},{4,5,6},{7,8,9}};
        std::cout << "Rank0: " << "\n";
        std::cout << T0 << "\n";
        MEL::Deep::Send(T0, 1, 99, comm);
        */

        /*
        StructA sA;
        MEL::Deep::Send<StructA, MEL::Deep::PointerHashMap,
                StructA_DeepCopy> (sA, 1, 99, comm);
        */
    }

    MEL::Barrier(comm);

    if(rank == 1){
        //Eigen::MatrixXd T1;
        //Eigen::ArrayXd T1(2);
        std::vector<double > T1;
        MEL::Deep::Recv<std::vector<double >, MEL::Deep::PointerHashMap,
                VecP_DeepCopy> (T1, 0, 99, comm);
        //MEL::Deep::Recv(T1, 0, 99, comm);
        std::cout << "Rank1: " << "\n";
        std::cout << T1[0] << "\n";
        
        /*
        Eigen::MatrixXd T1;
        //Eigen::ArrayXd T1(2);
        MEL::Deep::Recv(T1, 0, 99, comm);
        std::cout << "Rank1: " << "\n";
        std::cout << T1 << "\n";
        */

        /*
        StructA sA_r;
        MEL::Deep::Recv<StructA, MEL::Deep::PointerHashMap,
                StructA_DeepCopy> (sA_r, 0, 99, comm);
        */
    }

    MEL::Finalize();
    return 0;
}
