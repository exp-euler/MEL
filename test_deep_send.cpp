#include <vector>
#define MEL_IMPLEMENTATION
#include "MEL.hpp"
#include "MEL_deepcopy.hpp"

#include<Eigen/Dense>
#include <Eigen/Sparse>
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

// External - Global free deep-copy function for vectors of vectors
template<typename MSG> void VecVSTL_DeepCopy(std::vector<size_t> &obj, MSG &msg) {
    msg.packSTL(obj);
}

// External - Global free deep-copy function for vectors of vectors of vectors
template<typename MSG> void VecVVSTL_DeepCopy(std::vector<std::vector<double>> &obj, MSG &msg) {
    msg. template packSTL<std::vector<double>, VecVSTL_DeepCopy>(obj);
}


// #######################################

template<typename MSG>
void VecM_Eigen_DeepCopy(Eigen::MatrixXd &obj, MSG &msg) {
    msg.packMatrixXd(obj);
}

class ClassB {
    public:
        void set_vecB(std::vector<double> &vec) {
            vecB = vec;
        }

        std::vector<double> get_vecB() {
            return vecB;
        }

        void set_matB(std::vector<Eigen::MatrixXd> &mat) {
            matB = mat;
        }

        std::vector<Eigen::MatrixXd> get_matB() {
            return matB;
        }

        virtual void say_hi() {
            std::cout << "Here?" << "\n";
        }

    template<typename MSG>
    void DeepCopyB(MSG &msg) {
        msg.packSTL(vecB);
        msg. template packSTL<Eigen::MatrixXd, VecM_Eigen_DeepCopy>(matB);
    }

    //virtual ~ClassB(){};

    private:
        std::vector<double> vecB;
        std::vector<Eigen::MatrixXd> matB;
        std::vector<Eigen::MatrixXd> matBempty;
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
class SubClassB : public ClassB {
    public:
        void say_hi(){
            std::cout << "HI!" << "\n";
        }

        void say_hello(){
            std::cout << "HELLO!" << "\n";
        }

        template<typename MSG>
        void DeepCopy(MSG &msg) {
            this->DeepCopyB(msg);
        }
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

        /*
        SubClassB* p_objBB0(new SubClassB);
        SubClassB* p1_objBB0(new SubClassB);
        //SubClassB p_objBB0;
        std::vector<double> vec = {1.4578909, 2.1};
        Eigen::MatrixXd mat {{1,2,3},{4,5,6},{7,8,9}};
        std::vector<Eigen::MatrixXd> vecM = {mat};
        p_objBB0->set_vecB(vec);
        p_objBB0->set_matB(vecM);
        std::vector<SubClassB *> vec0_pClassesB = {p_objBB0};
        MEL::Deep::Send(vec0_pClassesB, 1, 17, comm);
        //SubClassB* casted = static_cast<SubClassB *> (p_objBB0);
        //MEL::Deep::Send(casted, 1, 17, comm);
        //MEL::Deep::Send(p_objBB0, 1, 17, comm);
        //p_objBB0->say_hello();
        std::cout << "Rank0: " << p_objBB0->get_vecB()[0] << "\n";

        std::cout<<"Rank0 Vptr = "<< *(int**)p_objBB0 <<std::endl;
        std::cout<<"Rank0 Vptr1 = "<< *(int**)p1_objBB0 <<std::endl;
        p_objBB0->say_hi();
        //casted->say_hi();

        //delete p_objBB0;
        */

        // Sending an Eigen Sparse matrix over MPI
        std::vector<Eigen::Triplet<double>> triplets;
        Eigen::SparseMatrix<double, Eigen::ColMajor> matrix; // should be ColMajor for SparseLU
        matrix.resize(6,6);
        triplets.emplace_back(0, 0, 1.0);
        triplets.emplace_back(1, 4, 2.0);
        triplets.emplace_back(3, 2, 3.0);
        triplets.emplace_back(5, 5, 4.0);
        matrix.setFromTriplets(triplets.begin(), triplets.end());
        std::cout << "Sparse Matrix: " << matrix << std::endl;

        std::cout << "valuePtr: " << matrix.valuePtr() << std::endl;
        std::cout << "innerIndex: " << matrix.innerIndexPtr() << std::endl;
        std::cout << "outerIndex: " << matrix.outerIndexPtr() << std::endl;
        MEL::Deep::Send<>(matrix, 1, 99, comm);

        /*
        size_t d0 = 4;
        size_t d1 = 4;
        size_t d2 = 4;
        std::vector<std::vector<size_t>> T0 {{d0}, {d1, d2}};
        std::cout << "Rank0: " << "\n";
        std::cout << (T0[0])[0] << "\n";
        MEL::Deep::Send<std::vector<std::vector<size_t>>, MEL::Deep::PointerHashMap, VecVSTL_DeepCopy> (T0, 1, 99, comm);
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
        Eigen::VectorXd T0 {{1,2,3}};
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

        /*
        std::vector<SubClassB *> vec1_pClassesB;
        SubClassB* casted1;
        SubClassB* casted2 = new SubClassB();
        //ClassB* p_objBB1;
        MEL::Deep::Recv(vec1_pClassesB, 0, 17, comm);
        casted1 = vec1_pClassesB[0];
        //SubClassB TEMP = *static_cast<SubClassB*>(casted1);
        ClassB* casted3 = new SubClassB(*casted1);
        //MEL::Deep::Recv(casted1, 0, 17, comm);
        //ClassB * p_objBB1 = casted1;
        //std::cout << "Rank1: " << p_objBB1->get_vecB()[0] << "\n";
        //p_objBB1->say_hello();
        //MEL::Deep::TESTING(casted1);
        std::cout << "Rank1: " << casted1->get_vecB()[0] << "\n";

        std::cout<<"Rank1 Vptr = "<< *(double**)casted1 <<std::endl;
        std::cout<<"Rank1 Vptr1 = "<< *(double**)casted2 <<std::endl;
        std::cout<<"Rank1 Vptr2 = "<< *(double**)casted3 <<std::endl;
        //std::cout<<"Rank1 Vptr1 g++ = "<< casted2->_vfptr_ClassB <<std::endl;
        casted3->say_hi();
        std::cout << "Rank1: " << casted3->get_vecB()[0] << "\n";
        std::cout << "Rank1: " << casted3->get_matB()[0] << "\n";

        //ClassB* temp = new ClassB();
        //std::cout << "Rank1: " << temp->_vptr << "\n";
        // Try to call say_hi() from ctable directly

        //casted1->_vptr = temp->_vptr;
        */

        Eigen::SparseMatrix<double, Eigen::ColMajor> matrix1; // should be ColMajor for SparseLU
        MEL::Deep::Recv(matrix1, 0, 99, comm);
        std::cout << "Rank1: " << "\n";
        std::cout << matrix1 << "\n";

        /*
        std::vector<std::vector<size_t>> T1;
        MEL::Deep::Recv<std::vector<std::vector<size_t>>, MEL::Deep::PointerHashMap,
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
        Eigen::VectorXd T1;
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
