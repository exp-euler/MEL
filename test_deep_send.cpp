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
template<typename MSG> void VecP_DeepCopy(double* &obj, MSG &msg) {
    msg.packPtr(obj);
}

// External - Global free deep-copy function for vectors of MatrixXd
template<typename MSG> void VecM_DeepCopy(Eigen::MatrixXd &obj, MSG &msg) {
    msg.packMatrixXd(obj);
}

// External - Global free deep-copy function for vectors of VectorXd
template<typename MSG> void VecVEigen_DeepCopy(Eigen::VectorXd &obj, MSG &msg) {
    msg.packVectorXd(obj);
}

// External - Global free deep-copy function for vectors of ArrayXXi
template<typename MSG> void VecArr2D_DeepCopy(Eigen::ArrayXXi &obj, MSG &msg) {
    msg.packArrayXXi(obj);
}

// External - Global free deep-copy function for vectors of ArrayXi
template<typename MSG> void VecArr1D_DeepCopy(Eigen::ArrayXi &obj, MSG &msg) {
    msg.packArrayXi(obj);
}

// External - Global free deep-copy function for vectors of vectors
template<typename MSG> void VecVSTL_DeepCopy(std::vector<double> &obj, MSG &msg) {
    msg.packSTL(obj);
}

// External - Global free deep-copy function for vectors of vectors of vectors
template<typename MSG> void VecVVSTL_DeepCopy(std::vector<std::vector<double>> &obj, MSG &msg) {
    msg. template packSTL<std::vector<double>, VecVSTL_DeepCopy>(obj);
}


// #######################################

class ClassB {
    public:
        void set_vecB(std::vector<double> &vec) {
            vecB = vec;
        }

        std::vector<double> get_vecB() {
            return vecB;
        }

    template<typename MSG>
    void DeepCopy(MSG &msg) {
        msg.packSTL(vecB);
    }

    private:
        std::vector<double> vecB;
};

class ClassA {
    public:
        ClassA() {}; // Default constructor
        ClassA(ClassB &obj) : objB(&obj) {}; // Assign constructor
        void set_vecA(std::vector<std::vector<double>> &vec) {
            vecA = vec;
        }

        std::vector<std::vector<double>> get_vecA() {
            return vecA;
        }

        ClassB* get_objB() {
            return objB;
        }

        template<typename MSG>
        void DeepCopy(MSG &msg) {
            msg. template packSTL<std::vector<double>, VecVSTL_DeepCopy>(vecA);
            msg.packPtr(objB);
        }
    private:
        std::vector<std::vector<double>> vecA;
        ClassB *objB;
};

// #######################################



int main(int argc, char *argv[]) {
    MEL::Init(argc,argv);

    MEL::Comm comm = MEL::Comm::Comm::WORLD;
    const int rank = MEL::CommRank(comm);

    // Send a class that has a member MatrixXd
    // Eigen::MatrixXd TEST {{1,2},{3,4}};
    //
    // Send by pointer to the class object as well as by value

    if(rank == 0){
        ClassB B0;
        std::vector<double> doubles_for_B = {1.2, 1.4, 1.789};
        B0.set_vecB(doubles_for_B);

        ClassA A0(B0);
        std::vector<std::vector<double>> doubles_for_A = {{2.3, 3.4},{2.1, 3.6}};
        A0.set_vecA(doubles_for_A);
        MEL::Deep::Send(A0, 1, 17, comm);

        /*
        double d0 = 2.4;
        std::vector<std::vector<double>> T0 {{d0}};
        std::cout << "Rank0: " << "\n";
        std::cout << (T0[0])[0] << "\n";
        MEL::Deep::Send<std::vector<std::vector<double>>, MEL::Deep::PointerHashMap, VecVSTL_DeepCopy> (T0, 1, 99, comm);
        */

        /*
        double d0 = 2.4;
        std::vector<std::vector<std::vector<double>>> T0 {{{d0}}};
        std::cout << "Rank0: " << "\n";
        std::cout << ((T0[0])[0])[0] << "\n";
        MEL::Deep::Send<std::vector<std::vector<std::vector<double>>>, MEL::Deep::PointerHashMap, VecVVSTL_DeepCopy> (T0, 1, 99, comm);
        */

        /*
        //std::vector<double> T0 {1.0, 3.5, 2.5};
        double d0 = 2.4;
        std::vector<double *> T0 {&d0};
        std::cout << "Rank0: " << "\n";
        std::cout << *T0[0] << "\n";
        MEL::Deep::Send<std::vector<double *>, MEL::Deep::PointerHashMap, VecP_DeepCopy> (T0, 1, 99, comm);
        */

        /*
        Eigen::MatrixXd M0 {{1,2,3},{4,5,6},{7,8,9}};
        std::vector<Eigen::MatrixXd> T0 {M0};
        std::cout << "Rank0: " << "\n";
        std::cout << T0[0] << "\n";
        MEL::Deep::Send<std::vector<Eigen::MatrixXd>, MEL::Deep::PointerHashMap, VecM_DeepCopy> (T0, 1, 99, comm);
        */

        /*
        Eigen::VectorXd VE0 {{1,2,3}};
        std::vector<Eigen::VectorXd> T0 {VE0};
        std::cout << "Rank0: " << "\n";
        std::cout << T0[0] << "\n";
        MEL::Deep::Send<std::vector<Eigen::VectorXd>, MEL::Deep::PointerHashMap, VecVEigen_DeepCopy> (T0, 1, 99, comm);
        */

        /*
        Eigen::ArrayXXi A0 {{1,2,3},{4,5,6},{7,8,9}};
        std::vector<Eigen::ArrayXXi> T0 {A0};
        std::cout << "Rank0: " << "\n";
        std::cout << T0[0] << "\n";
        MEL::Deep::Send<std::vector<Eigen::ArrayXXi>, MEL::Deep::PointerHashMap, VecArr2D_DeepCopy> (T0, 1, 99, comm);
        */

        /*
        Eigen::ArrayXi A0 {{1,2,3}};
        std::vector<Eigen::ArrayXi> T0 {A0};
        std::cout << "Rank0: " << "\n";
        std::cout << T0[0] << "\n";
        MEL::Deep::Send<std::vector<Eigen::ArrayXi>, MEL::Deep::PointerHashMap, VecArr1D_DeepCopy> (T0, 1, 99, comm);
        */

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
        ClassA A1;
        MEL::Deep::Recv(A1, 0, 17, comm);
        std::cout << "Rank1: " << (A1.get_objB())->get_vecB()[0] << "\n";

        /*
        std::vector<std::vector<double>> T1;
        MEL::Deep::Recv<std::vector<std::vector<double>>, MEL::Deep::PointerHashMap,
                VecVSTL_DeepCopy> (T1, 0, 99, comm);
        std::cout << "Rank1: " << "\n";
        std::cout << (T1[0])[0] << "\n";
        */

        /*
        std::vector<std::vector<std::vector<double>>> T1;
        MEL::Deep::Recv<std::vector<std::vector<std::vector<double>>>, MEL::Deep::PointerHashMap,
                VecVVSTL_DeepCopy> (T1, 0, 99, comm);
        std::cout << "Rank1: " << "\n";
        std::cout << ((T1[0])[0])[0] << "\n";
        */

        /*
        std::vector<double *> T1;
        MEL::Deep::Recv<std::vector<double *>, MEL::Deep::PointerHashMap,
                VecP_DeepCopy> (T1, 0, 99, comm);
        std::cout << "Rank1: " << "\n";
        std::cout << *T1[0] << "\n";
        */

        /*
        std::vector<Eigen::MatrixXd> T1;
        MEL::Deep::Recv<std::vector<Eigen::MatrixXd>, MEL::Deep::PointerHashMap,
                VecM_DeepCopy> (T1, 0, 99, comm);
        std::cout << "Rank1: " << "\n";
        std::cout << T1[0] << "\n";
        */

        /*
        std::vector<Eigen::ArrayXXi> T1;
        MEL::Deep::Recv<std::vector<Eigen::ArrayXXi>, MEL::Deep::PointerHashMap,
                VecArr2D_DeepCopy> (T1, 0, 99, comm);
        std::cout << "Rank1: " << "\n";
        std::cout << T1[0] << "\n";
        */

        /*
        std::vector<Eigen::ArrayXi> T1;
        MEL::Deep::Recv<std::vector<Eigen::ArrayXi>, MEL::Deep::PointerHashMap,
                VecArr1D_DeepCopy> (T1, 0, 99, comm);
        std::cout << "Rank1: " << "\n";
        std::cout << T1[0] << "\n";
        */

        /*
        std::vector<Eigen::VectorXd> T1;
        MEL::Deep::Recv<std::vector<Eigen::VectorXd>, MEL::Deep::PointerHashMap,
                VecVEigen_DeepCopy> (T1, 0, 99, comm);
        std::cout << "Rank1: " << "\n";
        std::cout << T1[0] << "\n";
        */
        
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
