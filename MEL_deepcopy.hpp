/*
The MIT License(MIT)

Copyright(c) 2016 Joss Whittle

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include "MEL.hpp"

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>
#include <list>
#include <fstream>
#include <unordered_map>

namespace MEL {
    namespace Deep {

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        class TransportSend {
        private:
            /// Members
            const int pid, tag;
            const MEL::Comm comm;

        public:
            static constexpr bool SOURCE = true;

            TransportSend(const int _pid, const int _tag, const MEL::Comm &_comm) : pid(_pid), tag(_tag), comm(_comm) {};

            template<typename T>
            inline void transport(T *&ptr, const int len) {
                MEL::Send(ptr, len, pid, tag, comm);
            };
        };

        class TransportRecv {
        private:
            /// Members
            const int pid, tag;
            const MEL::Comm comm;

        public:
            static constexpr bool SOURCE = false;

            TransportRecv(const int _pid, const int _tag, const MEL::Comm &_comm) : pid(_pid), tag(_tag), comm(_comm) {};

            template<typename T>
            inline void transport(T *&ptr, const int len) {
                MEL::Recv(ptr, len, pid, tag, comm);
            };
        };

        class TransportBcastRoot {
        private:
            /// Members
            const int root;
            const MEL::Comm comm;
            
        public:
            static constexpr bool SOURCE = true;

            TransportBcastRoot(const int _root, const MEL::Comm &_comm) : root(_root), comm(_comm) {};

            template<typename T>
            inline void transport(T *&ptr, const int len) {
                MEL::Bcast(ptr, len, root, comm);
            };
        };

        class TransportBcast {
        private:
            /// Members
            const int root;
            const MEL::Comm comm;

        public:
            static constexpr bool SOURCE = false;

            TransportBcast(const int _root, const MEL::Comm &_comm) : root(_root), comm(_comm) {};

            template<typename T>
            inline void transport(T *&ptr, const int len) {
                MEL::Bcast(ptr, len, root, comm);
            };
        };

        class TransportFileWrite {
        private:
            /// Members
            const MEL::File file;

        public:
            static constexpr bool SOURCE = true;

            TransportFileWrite(const MEL::File &_file) : file(_file) {};

            template<typename T>
            inline void transport(T *&ptr, const int len) {
                MEL::FileWrite(file, ptr, len);
            };
        };

        class TransportFileRead {
        private:
            /// Members
            const MEL::File file;

        public:
            static constexpr bool SOURCE = false;

            TransportFileRead(const MEL::File &_file) : file(_file) {};

            template<typename T>
            inline void transport(T *&ptr, const int len) {
                MEL::FileRead(file, ptr, len);
            };
        };

        class TransportSTLFileWrite {
        private:
            /// Members
            std::ofstream *file;

        public:
            static constexpr bool SOURCE = true;
            
            TransportSTLFileWrite(std::ofstream &_file) : file(&_file) {};

            template<typename T>
            inline void transport(T *&ptr, const int len) {
                const int num = len * sizeof(T);
                file->write((char*) ptr, num);
            };
        };

        class TransportSTLFileRead {
        private:
            /// Members
            std::ifstream *file;
               
        public:
            static constexpr bool SOURCE = false;

            TransportSTLFileRead(std::ifstream &_file) : file(&_file) {};

            template<typename T>
            inline void transport(T *&ptr, const int len) {
                const int num = len * sizeof(T);
                file->read((char *) ptr, num);
            };
        };

        class TransportBufferWrite {
        private:
            /// Members
            int offset, bufferSize;
            char *buffer;

        public:
            static constexpr bool SOURCE = true;

            TransportBufferWrite(char *_buffer, const int _bufferSize) : offset(0), buffer(_buffer), bufferSize(_bufferSize) {};

            template<typename T>
            inline void transport(T *&ptr, const int len) {
                const int num = len * sizeof(T);

                if ((offset + num) <= bufferSize) {
                    memcpy((void*) &buffer[offset], ptr, num);
                    offset += num;
                }
                else {
                    MEL::Abort(-1, "TransportBufferWrite : Offset longer than buffer...");
                }
            };
        };

        class TransportBufferRead {
        private:
            /// Members
            int offset, bufferSize;
            char *buffer;

        public:
            static constexpr bool SOURCE = false;

            TransportBufferRead(char *_buffer, const int _bufferSize) : offset(0), buffer(_buffer), bufferSize(_bufferSize) {};

            template<typename T>
            inline void transport(T *&ptr, const int len) {
                const int num = len * sizeof(T);

                if ((offset + num) <= bufferSize) {
                    memcpy((void*) ptr, &buffer[offset], num);
                    offset += num;
                }
                else {
                    MEL::Abort(-1, "TransportBufferRead : Offset longer than buffer...");
                }
            }
        };

        class NoTransport {
        public:
            static constexpr bool SOURCE = true; 
        
            explicit NoTransport(const int dummy) {};

            template<typename T>
            inline void transport(T *&ptr, const int len) {};
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        class PointerHashMap {
        private:
            std::unordered_map<void*, void*> pointerMap;

        public:
            // Pointer hashmap public interface

            // Returns true if oldPtr is found in the hash-map and sets ptr equal to the stored value
            // Otherwise returns false and ptr is unaltered
            template<typename T>
            inline bool find(T* oldPtr, T* &ptr) {
                // Is oldPtr already in the hashmap?
                const auto it = pointerMap.find((void*) oldPtr);
                if (it != pointerMap.end()) {
                    // If so set ptr equal to the value stored in the hashmap
                    ptr = (T*) it->second;
                    return true;
                }
                return false;
            };

            // Insert ptr into the hashmap using oldptr as the key
            template<typename T>
            inline void insert(T* oldPtr, T* ptr) {
                // The shift value to use for a type T
                pointerMap.insert(std::make_pair((void*) oldPtr, (void*) ptr));
            };
        };
        
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        template<typename TRANSPORT_METHOD, typename HASH_MAP = MEL::Deep::PointerHashMap>
        class Message;

        template<typename T>
        struct HasDeepCopyMethod {
            // This pseudo-type does not exist unless type U has the a member function of 
            // the desired form: template<typename MSG> void DeepCopy(MSG &msg)
            template<typename U, void(U::*)(MEL::Deep::Message<NoTransport>&)> struct SFINAE {};
            
            // If this succeeds Test<T> will be a function that returns char
            template<typename U> static char Test(SFINAE<U, &U::DeepCopy>*);
            // Otherwise Test<T> will return an int 
            template<typename U> static int  Test(...);

            // We can now test if type T has the desired member function by seeing if the result is the size
            // of a char or an int.
            static const bool Has = sizeof(Test<T>(0)) == sizeof(char);
        };

        template<typename T, typename R = void>
        using enable_if_deep = typename std::enable_if<HasDeepCopyMethod<T>::Has, R>::type;
        template<typename T, typename R = void>
        using enable_if_not_deep = typename std::enable_if<!(HasDeepCopyMethod<T>::Has), R>::type;
        template<typename T, typename R = void>
        using enable_if_pointer = typename std::enable_if<std::is_pointer<T>::value, R>::type;
        template<typename T, typename R = void>
        using enable_if_not_pointer = typename std::enable_if<!std::is_pointer<T>::value, R>::type;
        template<typename T, typename R = void>
        using enable_if_deep_not_pointer = typename std::enable_if<HasDeepCopyMethod<T>::Has && !std::is_pointer<T>::value, R>::type;
        template<typename T, typename R = void>
        using enable_if_not_deep_pointer = typename std::enable_if<!HasDeepCopyMethod<T>::Has && std::is_pointer<T>::value, R>::type;
        template<typename T, typename R = void>
        using enable_if_not_deep_not_pointer = typename std::enable_if<!HasDeepCopyMethod<T>::Has && !std::is_pointer<T>::value, R>::type;

        template<typename T> struct is_vector : public std::false_type {};
        template<typename T, typename A>
        struct is_vector<std::vector<T, A>> : public std::true_type{};

        template<typename T> struct is_list : public std::false_type {};
        template<typename T, typename A>
        struct is_list<std::list<T, A>> : public std::true_type{};
        //template <typename T>
        //using is_string = typename std::is_same<T, std::string>;
        
        template<typename T> struct is_eigen_matrix : public std::false_type {};
        template<>
        struct is_eigen_matrix<Eigen::MatrixXd> : public std::true_type{};
        template<typename T> struct is_eigen_sparse_matrix_col : public std::false_type {};
        template<>
        struct is_eigen_sparse_matrix_col<Eigen::SparseMatrix<double, Eigen::ColMajor>> : public std::true_type{};
        //template<typename Scalar, int RowsAtCompile, int ColsAtCompile>
        //struct is_eigen_matrix<Eigen::Matrix<Scalar, RowsAtCompile, ColsAtCompile>> : public std::true_type{};
        template<typename T> struct is_eigen_vector : public std::false_type {};
        template<>
        struct is_eigen_vector<Eigen::VectorXd> : public std::true_type{};
        template<typename T> struct is_eigen_array1D : public std::false_type {};
        template<>
        struct is_eigen_array1D<Eigen::ArrayXi> : public std::true_type{};
        template<typename T> struct is_eigen_array2D : public std::false_type {};
        template<>
        struct is_eigen_array2D<Eigen::ArrayXXi> : public std::true_type{};

        template<typename T, typename R = void>
        using enable_if_eigen_matrix = typename std::enable_if<is_eigen_matrix<T>::value, R>::type;
        template<typename T, typename R = void>
        using enable_if_eigen_sparse_matrix_col = typename std::enable_if<is_eigen_sparse_matrix_col<T>::value, R>::type;
        template<typename T, typename R = void>
        using enable_if_eigen_vector = typename std::enable_if<is_eigen_vector<T>::value, R>::type;
        template<typename T, typename R = void>
        using enable_if_eigen_array1D = typename std::enable_if<is_eigen_array1D<T>::value, R>::type;
        template<typename T, typename R = void>
        using enable_if_eigen_array2D= typename std::enable_if<is_eigen_array2D<T>::value, R>::type;

        template<typename T, typename R = void>
        using enable_if_stl = typename std::enable_if<is_vector<T>::value || is_list<T>::value, R>::type; //  || is_string<T>::value
        template<typename T, typename R = void>
        // TODO: Added not_eigen in definition, but not in name!
        using enable_if_not_pointer_not_stl = typename std::enable_if<!(is_vector<T>::value || is_list<T>::value || is_eigen_matrix<T>::value || is_eigen_sparse_matrix_col<T>::value || is_eigen_vector<T>::value || is_eigen_array1D<T>::value || is_eigen_array2D<T>::value) && !std::is_pointer<T>::value, R>::type; //  || is_string<T>::value
        template<typename T, typename R = void>
        using enable_if_deep_not_pointer_not_stl = typename std::enable_if<HasDeepCopyMethod<T>::Has && !(is_vector<T>::value || is_list<T>::value || is_eigen_matrix<T>::value || is_eigen_sparse_matrix_col<T>::value || is_eigen_vector<T>::value || is_eigen_array1D<T>::value || is_eigen_array2D<T>::value) && !std::is_pointer<T>::value, R>::type; //  || is_string<T>::value

        template<typename T, typename TRANSPORT_METHOD, typename HASH_MAP>
        using DEEP_FUNCTOR = void(*)(T&, MEL::Deep::Message<TRANSPORT_METHOD, HASH_MAP>&); 

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        template<typename TRANSPORT_METHOD, typename HASH_MAP>
        class Message {
        private:
            /// Members
            int              offset;
            TRANSPORT_METHOD transporter;
            HASH_MAP         pointerMap;
            
            template<typename P>
            inline enable_if_pointer<P> transport(P &ptr, const int len) {
                if (len > 0 && ptr != nullptr) {
                    typedef typename std::remove_pointer<P>::type T; // where P == T*, find T

                    offset += len * sizeof(T);
                    transporter.transport(ptr, len);
                }
            };

            template<typename T>
            inline enable_if_not_pointer<T> transport(T &obj) {
                T *ptr = &obj;
                transport(ptr, 1);
            };

            template<typename P>
            inline enable_if_pointer<P> transportAlloc(P &ptr, const int len) {
                if (!TRANSPORT_METHOD::SOURCE) {
                    typedef typename std::remove_pointer<P>::type T; // where P == T*, find T
                    ptr = (len > 0 && ptr != nullptr) ? MEL::MemAlloc<T>(len) : nullptr;
                }
                transport(ptr, len);
            };

        public:
            
            template<typename ...Args>
            Message(Args &&...args) : offset(0), transporter(std::forward<Args>(args)...) {};

            Message()                           = delete;
            Message(const Message &)            = delete;
            Message& operator=(const Message &) = delete;
            Message(Message &&)                 = delete;
            Message& operator=(Message &&)      = delete;

            inline int getOffset() const {
                return offset;
            };

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Transport API
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Object

            template<typename D>
            inline enable_if_deep<D> packVar(D &obj) {
                obj.DeepCopy(*this);
            };

            template<typename T, DEEP_FUNCTOR<T, TRANSPORT_METHOD, HASH_MAP> F>
            inline void packVar(T &obj) {
                F(obj, *this);
            };

            template<typename T>
            inline enable_if_not_deep<T> packRootVar(T &obj) {
                transport(obj);
            };

            template<typename T, DEEP_FUNCTOR<T, TRANSPORT_METHOD, HASH_MAP> F>
            inline void packRootVar(T &obj) {
                transport(obj);
                F(obj, *this);
            };

            template<typename D>
            inline enable_if_deep<D> packRootVar(D &obj) {
                transport(obj);
                obj.DeepCopy(*this);
            };

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Pointer

            template<typename T>
            inline enable_if_not_deep<T> packPtr(T* &ptr, int len = 1) {
                transportAlloc(ptr, len);
            };

            template<typename T, DEEP_FUNCTOR<T, TRANSPORT_METHOD, HASH_MAP> F>
            inline void packPtr(T* &ptr, int len = 1) {
                transportAlloc(ptr, len);
                /// Copy elements
                if (ptr != nullptr) {
                    for (int i = 0; i < len; ++i) {
                        F(ptr[i], *this);
                    }
                }
            };

            template<typename D>
            inline enable_if_deep<D> packPtr(D* &ptr, int len = 1) {
                transportAlloc(ptr, len);
                /// Copy elements
                if (ptr != nullptr) {
                    for (int i = 0; i < len; ++i) {
                        ptr[i].DeepCopy(*this);
                    }
                }
            };

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Shared Pointer

            template<typename T>
            inline enable_if_not_deep<T> packSharedPtr(T* &ptr, int len = 1) {
                T *oldPtr = ptr;
                if (pointerMap.find(oldPtr, ptr)) return;

                transportAlloc(ptr, len);
                pointerMap.insert(oldPtr, ptr);
            };

            template<typename T, DEEP_FUNCTOR<T, TRANSPORT_METHOD, HASH_MAP> F>
            inline void packSharedPtr(T* &ptr, int len = 1) {
                T *oldPtr = ptr;
                if (pointerMap.find(oldPtr, ptr)) return;

                transportAlloc(ptr, len);
                pointerMap.insert(oldPtr, ptr);

                /// Copy elements
                if (ptr != nullptr) {
                    for (int i = 0; i < len; ++i) {
                        F(ptr[i], *this);
                    }
                }
            };

            template<typename D>
            inline enable_if_deep<D> packSharedPtr(D* &ptr, int len = 1) {
                D *oldPtr = ptr;
                if (pointerMap.find(oldPtr, ptr)) return;

                transportAlloc(ptr, len);
                pointerMap.insert(oldPtr, ptr);

                /// Copy elements
                if (ptr != nullptr) {
                    for (int i = 0; i < len; ++i) {
                        ptr[i].DeepCopy(*this);
                    }
                }
            };

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Root Pointer

            template<typename T>
            inline enable_if_not_deep<T> packRootPtr(T* &ptr, int len = 1) {
                // Explicitly transport the pointer value for the root node
                size_t addr = (size_t) ptr;
                transport(addr);
                ptr = (T*) addr;

                T *oldPtr = ptr;
                if (pointerMap.find(oldPtr, ptr)) return;

                transportAlloc(ptr, len);
                pointerMap.insert(oldPtr, ptr);
            };

            template<typename T, DEEP_FUNCTOR<T, TRANSPORT_METHOD, HASH_MAP> F>
            inline void packRootPtr(T* &ptr, int len = 1) {
                // Explicitly transport the pointer value for the root node
                size_t addr = (size_t) ptr;
                transport(addr);
                ptr = (T*) addr;

                T *oldPtr = ptr;
                if (pointerMap.find(oldPtr, ptr)) return;

                transportAlloc(ptr, len);
                pointerMap.insert(oldPtr, ptr);
                
                /// Copy elements
                if (ptr != nullptr) {
                    for (int i = 0; i < len; ++i) {
                        F(ptr[i], *this);
                    }
                }
            };

            template<typename D>
            inline enable_if_deep<D> packRootPtr(D* &ptr, int len = 1) {
                // Explicitly transport the pointer value for the root node
                size_t addr = (size_t) ptr;
                transport(addr);
                ptr = (D*) addr;
                
                D *oldPtr = ptr;
                if (pointerMap.find(oldPtr, ptr)) return;

                transportAlloc(ptr, len);
                pointerMap.insert(oldPtr, ptr);

                /// Copy elements
                if (ptr != nullptr) {
                    for (int i = 0; i < len; ++i) {
                        ptr[i].DeepCopy(*this);
                    }
                }
            };

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // STL

            inline void packSTL(std::string &obj) {
                int len;
                if (TRANSPORT_METHOD::SOURCE) {
                    len = obj.size();
                    transport(len);
                }
                else {
                    transport(len);
                    new (&obj) std::string(len, ' ');
                }

                char *p = &obj[0];
                if (len > 0) transport(p, len);
            };

            template<typename T>
            inline enable_if_not_deep<T> packSTL(std::vector<T> &obj) {
                int len = obj.size();
                if (!TRANSPORT_METHOD::SOURCE) {
                    new (&obj) std::vector<T>(len, T());
                    for (int i = 0; i < len; ++i) (&obj[i])->~T();
                }

                T *p = &obj[0];
                if (len > 0) transport(p, len);
            };

            template<typename T, DEEP_FUNCTOR<T, TRANSPORT_METHOD, HASH_MAP> F>
            inline void packSTL(std::vector<T> &obj) {
                int len = obj.size();
                if (!TRANSPORT_METHOD::SOURCE) {
                    new (&obj) std::vector<T>(len);
                    for (int i = 0; i < len; ++i) (&obj[i])->~T();
                }

                T *p = &obj[0];
                if (len > 0) transport(p, len);

                /// Copy content
                for (int i = 0; i < len; ++i) {
                    F(obj[i], *this);
                }
            };

            template<typename D>
            inline enable_if_deep<D> packSTL(std::vector<D> &obj) {
                int len = obj.size();
                if (!TRANSPORT_METHOD::SOURCE) {
                    new (&obj) std::vector<D>(len);
                    for (int i = 0; i < len; ++i) (&obj[i])->~D();
                }

                D *p = &obj[0];
                if (len > 0) transport(p, len);
                /// Copy content
                for (int i = 0; i < len; ++i) {
                    obj[i].DeepCopy(*this);
                }
            };

            template<typename T>
            inline enable_if_not_deep<T> packSTL(std::list<T> &obj) {
                int len;
                if (TRANSPORT_METHOD::SOURCE) {
                    len = obj.size(); transport(len);
                }
                else {
                    transport(len); new (&obj) std::list<T>(len);
                }
                /// Copy content
                for (auto it = obj.begin(); it != obj.end(); ++it) {
                    if (!TRANSPORT_METHOD::SOURCE) (it)->~T();
                    this->packRootVar(*it);
                }
            };

            template<typename T, DEEP_FUNCTOR<T, TRANSPORT_METHOD, HASH_MAP> F>
            inline void packSTL(std::list<T> &obj) {
                int len;
                if (TRANSPORT_METHOD::SOURCE) {
                    len = obj.size(); transport(len);
                }
                else {
                    transport(len); new (&obj) std::list<T>(len);
                }
                /// Copy content
                for (auto it = obj.begin(); it != obj.end(); ++it) {
                    if (!TRANSPORT_METHOD::SOURCE) (it)->~T();
                    this->packRootVar<T, F>(*it);
                }
            };

            template<typename D>
            inline enable_if_deep<D> packSTL(std::list<D> &obj) {
                int len;
                if (TRANSPORT_METHOD::SOURCE) {
                    len = obj.size(); transport(len);
                }
                else {
                    transport(len); new (&obj) std::list<D>(len);
                }

                /// Copy content
                for (auto it = obj.begin(); it != obj.end(); ++it) {
                    if (!TRANSPORT_METHOD::SOURCE) (it)->~D();
                    this->packRootVar(*it);
                }
            };

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Root STL

            // ###### EDITED #######
            template<typename T>
            inline enable_if_not_deep_pointer<T> packRootSTL(std::vector<T> &obj) {
                int len;
                if (TRANSPORT_METHOD::SOURCE) {
                    len = obj.size(); transport(len);
                }
                else {
                    transport(len); obj.resize(len);
                    for (int i = 0; i < len; ++i) (&obj[i])->~T();
                }

                T *p = &obj[0];
                if (len > 0) transport(p, len);
                // ###### ADDED #######
                /// Copy content
                for (int i = 0; i < len; ++i) {
                    packPtr(obj[i]);
                }
                // ###### ADDED #######
            };

            template<typename T>
            inline enable_if_not_deep_not_pointer<T> packRootSTL(std::vector<T> &obj) {
                int len;
                if (TRANSPORT_METHOD::SOURCE) {
                    int rank = MEL::CommRank(MEL::Comm::WORLD);
                    len = obj.size(); transport(len);
                }
                else {
                    transport(len); obj.resize(len);
                    for (int i = 0; i < len; ++i) (&obj[i])->~T();
                }

                T *p = &obj[0];
                if (len > 0) transport(p, len);
            };
            // ###### EDITED #######

            template<typename T, DEEP_FUNCTOR<T, TRANSPORT_METHOD, HASH_MAP> F>
            inline void packRootSTL(std::vector<T> &obj) {
                int len;
                if (TRANSPORT_METHOD::SOURCE) {
                    len = obj.size(); transport(len);
                }
                else {
                    transport(len); obj.resize(len);
                    for (int i = 0; i < len; ++i) (&obj[i])->~T();
                }

                T *p = &obj[0];
                if (len > 0) transport(p, len);
                /// Copy content
                for (int i = 0; i < len; ++i) {
                    F(obj[i], *this);
                }
            };

            template<typename D>
            inline enable_if_deep<D> packRootSTL(std::vector<D> &obj) {
                int len;
                if (TRANSPORT_METHOD::SOURCE) {
                    len = obj.size(); transport(len);
                }
                else {
                    transport(len); obj.resize(len);
                    for (int i = 0; i < len; ++i) (&obj[i])->~D();
                }

                D *p = &obj[0];
                if (len > 0) transport(p, len);
                /// Copy content
                for (int i = 0; i < len; ++i) {
                    obj[i].DeepCopy(*this);
                }
            };

            template<typename T>
            inline enable_if_not_deep<T> packRootSTL(std::list<T> &obj) {
                int len;
                if (TRANSPORT_METHOD::SOURCE) {
                    len = obj.size(); transport(len);
                }
                else {
                    transport(len); obj.resize(len);
                }

                /// Copy content
                for (auto it = obj.begin(); it != obj.end(); ++it) {
                    if (!TRANSPORT_METHOD::SOURCE) (it)->~T();
                    this->packRootVar(*it);
                }
            };

            template<typename T, DEEP_FUNCTOR<T, TRANSPORT_METHOD, HASH_MAP> F>
            inline void packRootSTL(std::list<T> &obj) {
                int len;
                if (TRANSPORT_METHOD::SOURCE) {
                    len = obj.size(); transport(len);
                }
                else {
                    transport(len); obj.resize(len);
                }

                /// Copy content
                for (auto it = obj.begin(); it != obj.end(); ++it) {
                    if (!TRANSPORT_METHOD::SOURCE) (it)->~T();
                    this->packRootVar<T, F>(*it);
                }
            };

            template<typename D>
            inline enable_if_deep<D> packRootSTL(std::list<D> &obj) {
                int len;
                if (TRANSPORT_METHOD::SOURCE) {
                    len = obj.size(); transport(len);
                }
                else {
                    transport(len); obj.resize(len);
                }

                /// Copy content
                for (auto it = obj.begin(); it != obj.end(); ++it) {
                    if (!TRANSPORT_METHOD::SOURCE) (it)->~D();
                    this->packRootVar(*it);
                }
            };

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Root Eigen

            //template<typename T>
            //inline enable_if_not_deep<T> packRootMatrixXd(Eigen::Matrix<T, T_rows, T_cols> &obj) {
            inline enable_if_not_deep<double> packRootMatrixXd(Eigen::MatrixXd &obj) {
                int rows;
                int cols;
                if (TRANSPORT_METHOD::SOURCE) {
                    int rank = MEL::CommRank(MEL::Comm::WORLD);
                    rows = obj.rows(); transport(rows);
                    cols = obj.cols(); transport(cols);
                }
                else {
                    transport(rows); transport(cols);
                    obj.resize(rows, cols);
                    //for (int i = 0; i < rows+cols; ++i) (&obj[i])->~T();
                    //for (int i = 0; i < rows+cols; ++i) (&(obj(0,0))+i)->~double();
                }

                //T *p = &obj[0];
                double *p = &obj(0,0);
                if (rows+cols > 0) transport(p, rows*cols);
            };

            inline enable_if_not_deep<double> packRootSparseMatrix(Eigen::SparseMatrix<double, Eigen::ColMajor> &obj) {
                int rows;
                int cols;
                int nnz;
                if (TRANSPORT_METHOD::SOURCE) {
                    rows = obj.rows(); transport(rows);
                    cols = obj.cols(); transport(cols);
                    nnz = obj.nonZeros(); transport(nnz);
                    assert(cols == obj.outerSize());
                }
                else {
                    transport(rows); transport(cols); transport(nnz);
                    obj.resize(rows, cols);
                    obj.reserve(nnz);
                }

                //T *p = &obj[0];
                double *vPtr = obj.valuePtr();
                int *iPtr = obj.innerIndexPtr();
                int *oPtr = obj.outerIndexPtr();
                if (nnz > 0) {transport(vPtr, nnz); transport(iPtr, nnz);}
                if (cols > 0) transport(oPtr, cols);
                if (!TRANSPORT_METHOD::SOURCE) {
                    obj.outerIndexPtr()[cols] = nnz;
                }
            };

            inline enable_if_not_deep<double> packRootVectorXd(Eigen::VectorXd &obj) {
                int rows; // Because it is saved as an Eigen Matrix
                if (TRANSPORT_METHOD::SOURCE) {
                    rows = obj.rows(); transport(rows);
                }
                else {
                    transport(rows);
                    obj.resize(rows);
                }

                double *p = &obj(0,0);
                if (rows > 0) transport(p, rows);
            };

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Eigen

            //template<typename T>
            //inline enable_if_not_deep<T> packMatrixXd(Eigen::Matrix<T, T_rows, T_cols> &obj) {
            inline enable_if_not_deep<double> packMatrixXd(Eigen::MatrixXd &obj) {
                int rows;
                int cols;
                if (TRANSPORT_METHOD::SOURCE) {
                    rows = obj.rows(); transport(rows);
                    cols = obj.cols(); transport(cols);
                }
                else {
                    transport(rows); transport(cols);
                    new (&obj) Eigen::MatrixXd;
                    obj.resize(rows, cols);
                }

                double *p = &obj(0,0);
                if (rows+cols > 0) transport(p, rows*cols);
            };

            inline enable_if_not_deep<double> packVectorXd(Eigen::VectorXd &obj) {
                int rows; // Because it is saved as an Eigen Matrix
                if (TRANSPORT_METHOD::SOURCE) {
                    rows = obj.rows(); transport(rows);
                }
                else {
                    transport(rows);
                    new (&obj) Eigen::VectorXd;
                    obj.resize(rows);
                }

                double *p = &obj(0,0);
                if (rows > 0) transport(p, rows);
            };

            inline enable_if_not_deep<double> packArrayXXi(Eigen::ArrayXXi &obj) {
                int rows;
                int cols;
                if (TRANSPORT_METHOD::SOURCE) {
                    rows = obj.rows(); transport(rows);
                    cols = obj.cols(); transport(cols);
                }
                else {
                    transport(rows); transport(cols);
                    new (&obj) Eigen::ArrayXXi;
                    obj.resize(rows, cols);
                }

                int *p = &obj(0,0);
                if (rows+cols > 0) transport(p, rows*cols);
            };

            inline enable_if_not_deep<double> packArrayXi(Eigen::ArrayXi &obj) {
                int rows; // Because it is saved as an Eigen Matrix
                if (TRANSPORT_METHOD::SOURCE) {
                    rows = obj.rows(); transport(rows);
                }
                else {
                    transport(rows);
                    new (&obj) Eigen::ArrayXi;
                    obj.resize(rows);
                }

                int *p = &obj(0,0);
                if (rows > 0) transport(p, rows);
            };

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Shorthand Overloads

            template<typename T>
            inline Message<TRANSPORT_METHOD, HASH_MAP>& operator&(std::vector<T> &obj) {
                packSTL(obj);
                return *this;
            };

            template<typename T>
            inline Message<TRANSPORT_METHOD, HASH_MAP>& operator&(std::list<T> &obj) {
                packSTL(obj);
                return *this;
            };

            template<typename T>
            inline Message<TRANSPORT_METHOD, HASH_MAP>& operator&(T &obj) {
                packVar(obj);
                return *this;
            };

            inline Message<TRANSPORT_METHOD, HASH_MAP>& operator&(std::string &obj) {
                packSTL(obj);
                return *this;
            };
        };

#define TEMPLATE_MAT_E template<typename M, typename HASH_MAP = MEL::Deep::PointerHashMap>
#define TEMPLATE_VEC_E template<typename V, typename HASH_MAP = MEL::Deep::PointerHashMap>
#define TEMPLATE_STL template<typename S, typename HASH_MAP = MEL::Deep::PointerHashMap>
#define TEMPLATE_T   template<typename T, typename HASH_MAP = MEL::Deep::PointerHashMap>
#define TEMPLATE_P   template<typename P, typename HASH_MAP = MEL::Deep::PointerHashMap>

#define TEMPLATE_STL_F(transport_method) template<typename S, typename HASH_MAP, DEEP_FUNCTOR<typename S::value_type, transport_method, HASH_MAP> F>
#define TEMPLATE_T_F(transport_method)   template<typename T, typename HASH_MAP, DEEP_FUNCTOR<T, transport_method, HASH_MAP> F>
#define TEMPLATE_P_F(transport_method)   template<typename P, typename HASH_MAP, DEEP_FUNCTOR<typename std::remove_pointer<P>::type, transport_method, HASH_MAP> F>

#define TEMPLATE_STL_F2(transport_method1, transport_method2) template<typename S, typename HASH_MAP, DEEP_FUNCTOR<typename S::value_type, transport_method1, HASH_MAP> F1,                \
                                                                                                      DEEP_FUNCTOR<typename S::value_type, transport_method2, HASH_MAP> F2>
#define TEMPLATE_T_F2(transport_method1, transport_method2)   template<typename T, typename HASH_MAP, DEEP_FUNCTOR<T, transport_method1, HASH_MAP> F1,                                     \
                                                                                                      DEEP_FUNCTOR<T, transport_method2, HASH_MAP> F2>
#define TEMPLATE_P_F2(transport_method1, transport_method2)   template<typename P, typename HASH_MAP, DEEP_FUNCTOR<typename std::remove_pointer<P>::type, transport_method1, HASH_MAP> F1, \
                                                                                                      DEEP_FUNCTOR<typename std::remove_pointer<P>::type, transport_method2, HASH_MAP> F2>

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Buffer Size
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer / Length

        TEMPLATE_P
        inline enable_if_pointer<P, int> BufferSize(P &ptr, const int len) {
            Message<NoTransport, HASH_MAP> msg(0);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);
            return msg.getOffset();
        };

        TEMPLATE_P_F(NoTransport)
        inline enable_if_pointer<P, int> BufferSize(P &ptr, const int len) {
            typedef typename std::remove_pointer<P>::type T;
            Message<NoTransport, HASH_MAP> msg(0);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);
            return msg.getOffset();
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer

        TEMPLATE_P
        inline enable_if_pointer<P, int> BufferSize(P &ptr) {
            Message<NoTransport, HASH_MAP> msg(0);
            msg.packRootPtr(ptr);
            return msg.getOffset();
        };

        TEMPLATE_P_F(NoTransport)
        inline enable_if_pointer<P, int> BufferSize(P &ptr) {
            typedef typename std::remove_pointer<P>::type T;
            Message<NoTransport, HASH_MAP> msg(0);
            msg. template packRootPtr<T, F>(ptr);
            return msg.getOffset();
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // STL

        TEMPLATE_STL
        inline enable_if_stl<S, int> BufferSize(S &obj) {
            Message<NoTransport, HASH_MAP> msg(0);
            msg.packRootSTL(obj);
            return msg.getOffset();
        };

        TEMPLATE_STL_F(NoTransport)
        inline enable_if_stl<S, int> BufferSize(S &obj) {
            typedef typename S::value_type T;
            Message<NoTransport, HASH_MAP> msg(0);
            msg. template packRootSTL<T, F>(obj);
            return msg.getOffset();
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Object

        TEMPLATE_T
        inline enable_if_not_pointer_not_stl<T, int> BufferSize(T &obj) {
            Message<NoTransport, HASH_MAP> msg(0);
            msg.packRootVar(obj);
            return msg.getOffset();
        };

        TEMPLATE_T_F(NoTransport)
        inline enable_if_not_pointer_not_stl<T, int> BufferSize(T &obj) {
            Message<NoTransport, HASH_MAP> msg(0);
            msg. template packRootVar<T, F>(obj);
            return msg.getOffset();
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Send
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer / Length

        TEMPLATE_P
        inline enable_if_pointer<P> Send(P &ptr, int const &len, const int dst, const int tag, const Comm &comm) {
            Message<TransportSend, HASH_MAP> msg(dst, tag, comm);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);
        };

        TEMPLATE_P_F(TransportSend)
        inline enable_if_pointer<P> Send(P &ptr, int const &len, const int dst, const int tag, const Comm &comm) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportSend, HASH_MAP> msg(dst, tag, comm);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedSend(P &ptr, int const &len, const int dst, const int tag, const Comm &comm, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);

            MEL::Deep::Send(buffer, msg.getOffset(), dst, tag, comm);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferWrite)
        inline enable_if_pointer<P> BufferedSend(P &ptr, int const &len, const int dst, const int tag, const Comm &comm, const int bufferSize) {
            typedef typename std::remove_pointer<P>::type T;
            
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);
            
            MEL::Deep::Send(buffer, msg.getOffset(), dst, tag, comm);
            MEL::MemFree(buffer);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedSend(P &ptr, int const &len, const int dst, const int tag, const Comm &comm) {
            MEL::Deep::BufferedSend(ptr, len, dst, tag, comm, MEL::Deep::BufferSize(ptr, len));
        };

        TEMPLATE_P_F(TransportBufferWrite)
        inline enable_if_pointer<P> BufferedSend(P &ptr, int const &len, const int dst, const int tag, const Comm &comm) {
            MEL::Deep::BufferedSend<P, HASH_MAP, F>(ptr, len, dst, tag, comm, MEL::Deep::BufferSize<P, HASH_MAP, F>(ptr, len));
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer

        TEMPLATE_P
        inline enable_if_pointer<P> Send(const P &ptr, const int dst, const int tag, const Comm &comm) {
            Send((P) ptr, dst, tag, comm);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> Send(P &ptr, const int dst, const int tag, const Comm &comm) {
            Message<TransportSend, HASH_MAP> msg(dst, tag, comm);
            msg.packRootPtr(ptr);
        };

        TEMPLATE_P_F(TransportSend)
        inline enable_if_pointer<P> Send(P &ptr, const int dst, const int tag, const Comm &comm) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportSend, HASH_MAP> msg(dst, tag, comm);
            msg. template packRootPtr<T, F>(ptr);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedSend(P &ptr, const int dst, const int tag, const Comm &comm, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootPtr(ptr);

            MEL::Deep::Send(buffer, msg.getOffset(), dst, tag, comm);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferWrite)
        inline enable_if_pointer<P> BufferedSend(P &ptr, const int dst, const int tag, const Comm &comm, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            typedef typename std::remove_pointer<P>::type T;
            msg. template packRootPtr<T, F>(ptr);

            MEL::Deep::Send(buffer, msg.getOffset(), dst, tag, comm);
            MEL::MemFree(buffer);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedSend(P &ptr, const int dst, const int tag, const Comm &comm) {
            MEL::Deep::BufferedSend(ptr, dst, tag, comm, MEL::Deep::BufferSize(ptr));
        };

        TEMPLATE_P_F(TransportBufferWrite)
        inline enable_if_pointer<P> BufferedSend(P &ptr, const int dst, const int tag, const Comm &comm) {
            MEL::Deep::BufferedSend<P, HASH_MAP, F>(ptr, dst, tag, comm, MEL::Deep::BufferSize<P, HASH_MAP, F>(ptr));
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // STL 

        TEMPLATE_STL
        inline enable_if_stl<S> Send(S &obj, const int dst, const int tag, const Comm &comm) {
            Message<TransportSend, HASH_MAP> msg(dst, tag, comm);
            msg.packRootSTL(obj);
        };

        TEMPLATE_STL_F(TransportSend)
        inline enable_if_stl<S> Send(S &obj, const int dst, const int tag, const Comm &comm) {
            Message<TransportSend, HASH_MAP> msg(dst, tag, comm);
            typedef typename S::value_type T;
            msg. template packRootSTL<T, F>(obj);
        };

        TEMPLATE_STL
        inline enable_if_stl<S> BufferedSend(S &obj, const int dst, const int tag, const Comm &comm, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootSTL(obj);

            MEL::Deep::Send(buffer, msg.getOffset(), dst, tag, comm);
            MEL::MemFree(buffer);
        };

        TEMPLATE_STL_F(TransportBufferWrite)
        inline enable_if_stl<S> BufferedSend(S &obj, const int dst, const int tag, const Comm &comm, const int bufferSize) {
            typedef typename S::value_type T;
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootSTL<T, F>(obj);

            MEL::Deep::Send(buffer, msg.getOffset(), dst, tag, comm);
            MEL::MemFree(buffer);
        };

        TEMPLATE_STL
        inline enable_if_stl<S> BufferedSend(S &obj, const int dst, const int tag, const Comm &comm) {
            MEL::Deep::BufferedSend(obj, dst, tag, comm, MEL::Deep::BufferSize(obj));
        };

        TEMPLATE_STL_F(TransportBufferWrite)
        inline enable_if_stl<S> BufferedSend(S &obj, const int dst, const int tag, const Comm &comm) {
            MEL::Deep::BufferedSend<S, HASH_MAP, F>(obj, dst, tag, comm, MEL::Deep::BufferSize<S, HASH_MAP, F>(obj));
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Root MatrixXd

        TEMPLATE_MAT_E
        inline enable_if_eigen_matrix<M> Send(M &obj, const int dst, const int tag, const Comm &comm) {
            Message<TransportSend, HASH_MAP> msg(dst, tag, comm);
            msg.packRootMatrixXd(obj);
        };

        // Root SparseMatrix

        TEMPLATE_MAT_E
        inline enable_if_eigen_sparse_matrix_col<M> Send(M &obj, const int dst, const int tag, const Comm &comm) {
            Message<TransportSend, HASH_MAP> msg(dst, tag, comm);
            msg.packRootSparseMatrix(obj);
        };

        // Root VectorXd

        TEMPLATE_VEC_E
        inline enable_if_eigen_vector<V> Send(V &obj, const int dst, const int tag, const Comm &comm) {
            Message<TransportSend, HASH_MAP> msg(dst, tag, comm);
            msg.packRootVectorXd(obj);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Object

        TEMPLATE_T
        inline enable_if_not_pointer_not_stl<T> Send(T &obj, const int dst, const int tag, const Comm &comm) {
            Message<TransportSend, HASH_MAP> msg(dst, tag, comm);
            msg.packRootVar(obj);
        };

        TEMPLATE_T_F(TransportSend)
        inline enable_if_not_pointer_not_stl<T> Send(T &obj, const int dst, const int tag, const Comm &comm) {
            Message<TransportSend, HASH_MAP> msg(dst, tag, comm);
            msg. template packRootVar<T, F>(obj);
        };

        TEMPLATE_T
        inline enable_if_deep_not_pointer_not_stl<T> BufferedSend(T &obj, const int dst, const int tag, const Comm &comm, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(obj);

            MEL::Deep::Send(buffer, msg.getOffset(), dst, tag, comm);
            MEL::MemFree(buffer);
        };

        TEMPLATE_T_F(TransportBufferWrite)
        inline enable_if_not_pointer_not_stl<T> BufferedSend(T &obj, const int dst, const int tag, const Comm &comm, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootVar<T, F>(obj);

            MEL::Deep::Send(buffer, msg.getOffset(), dst, tag, comm);
            MEL::MemFree(buffer);
        };

        TEMPLATE_T
        inline enable_if_deep_not_pointer_not_stl<T> BufferedSend(T &obj, const int dst, const int tag, const Comm &comm) {
            MEL::Deep::BufferedSend(obj, dst, tag, comm, MEL::Deep::BufferSize(obj));
        };

        TEMPLATE_T_F(TransportBufferWrite)
        inline enable_if_not_pointer_not_stl<T> BufferedSend(T &obj, const int dst, const int tag, const Comm &comm) {
            MEL::Deep::BufferedSend<T, HASH_MAP, F>(obj, dst, tag, comm, MEL::Deep::BufferSize<T, HASH_MAP, F>(obj));
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Recv
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer / Length

        TEMPLATE_P
        inline enable_if_pointer<P> Recv(P &ptr, int &len, const int src, const int tag, const Comm &comm) {
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);
        };

        TEMPLATE_P_F(TransportRecv)
        inline enable_if_pointer<P> Recv(P &ptr, int &len, const int src, const int tag, const Comm &comm) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> Recv(P &ptr, int const &len, const int src, const int tag, const Comm &comm) {
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            int _len = len; 
            msg.packRootVar(_len);
            if (len != _len) MEL::Exit(-1, "MEL::Deep::Recv(ptr, len) const int len provided does not match incomming message size.");
            msg.packRootPtr(ptr, _len);
        };

        TEMPLATE_P_F(TransportRecv)
        inline enable_if_pointer<P> Recv(P &ptr, int const &len, const int src, const int tag, const Comm &comm) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            int _len = len;
            msg.packRootVar(_len);
            if (len != _len) MEL::Exit(-1, "MEL::Deep::Recv(ptr, len) const int len provided does not match incomming message size.");
            msg. template packRootPtr<T, F>(ptr, _len);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedRecv(P &ptr, int &len, const int src, const int tag, const Comm &comm) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::Recv(buffer, bufferSize, src, tag, comm);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferRead)
        inline enable_if_pointer<P> BufferedRecv(P &ptr, int &len, const int src, const int tag, const Comm &comm) {
            typedef typename std::remove_pointer<P>::type T;
            
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::Recv(buffer, bufferSize, src, tag, comm);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedRecv(P &ptr, int const &len, const int src, const int tag, const Comm &comm) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::Recv(buffer, bufferSize, src, tag, comm);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            int _len = len;
            msg.packRootVar(_len);
            if (len != _len) MEL::Exit(-1, "MEL::Deep::BufferedRecv(ptr, len) const int len provided does not match incomming message size.");
            msg.packRootPtr(ptr, _len);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferRead)
        inline enable_if_pointer<P> BufferedRecv(P &ptr, int const &len, const int src, const int tag, const Comm &comm) {
            typedef typename std::remove_pointer<P>::type T;
            
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::Recv(buffer, bufferSize, src, tag, comm);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            int _len = len;
            msg.packRootVar(_len);
            if (len != _len) MEL::Exit(-1, "MEL::Deep::BufferedRecv(ptr, len) const int len provided does not match incomming message size.");
            msg. template packRootPtr<T, F>(ptr, _len);

            MEL::MemFree(buffer);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer

        TEMPLATE_P
        inline enable_if_pointer<P> Recv(P &ptr, const int src, const int tag, const Comm &comm) {
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            
            msg.packRootPtr(ptr);
        };

        TEMPLATE_P_F(TransportRecv)
        inline enable_if_pointer<P> Recv(P &ptr, const int src, const int tag, const Comm &comm) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            msg. template packRootPtr<T, F>(ptr);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedRecv(P &ptr, const int src, const int tag, const Comm &comm) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::Recv(buffer, bufferSize, src, tag, comm);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootPtr(ptr);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferRead)
        inline enable_if_pointer<P> BufferedRecv(P &ptr, const int src, const int tag, const Comm &comm) {
            typedef typename std::remove_pointer<P>::type T;
            
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::Recv(buffer, bufferSize, src, tag, comm);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootPtr<T, F>(ptr);

            MEL::MemFree(buffer);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // STL

        TEMPLATE_STL
        inline enable_if_stl<S> Recv(S &obj, const int src, const int tag, const Comm &comm) {
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            msg.packRootSTL(obj);
        };

        TEMPLATE_STL_F(TransportRecv)
        inline enable_if_stl<S> Recv(S &obj, const int src, const int tag, const Comm &comm) {
            typedef typename S::value_type T;
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            msg. template packRootSTL<T, F>(obj);
        };

        TEMPLATE_STL
        inline enable_if_stl<S> BufferedRecv(S &obj, const int src, const int tag, const Comm &comm) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::Recv(buffer, bufferSize, src, tag, comm);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootSTL(obj);

            MEL::MemFree(buffer);
        };

        TEMPLATE_STL_F(TransportBufferRead)
        inline enable_if_stl<S> BufferedRecv(S &obj, const int src, const int tag, const Comm &comm) {
            typedef typename S::value_type T;
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::Recv(buffer, bufferSize, src, tag, comm);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootSTL<T, F>(obj);

            MEL::MemFree(buffer);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Root MatrixXd

        TEMPLATE_MAT_E
        inline enable_if_eigen_matrix<M> Recv(M &obj, const int src, const int tag, const Comm &comm) {
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            msg.packRootMatrixXd(obj);
        };

        // Root SparseMatrix

        TEMPLATE_MAT_E
        inline enable_if_eigen_sparse_matrix_col<M> Recv(M &obj, const int src, const int tag, const Comm &comm) {
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            msg.packRootSparseMatrix(obj);
        };

        // Root VectorXd

        TEMPLATE_VEC_E
        inline enable_if_eigen_vector<V> Recv(V &obj, const int src, const int tag, const Comm &comm) {
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            msg.packRootVectorXd(obj);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Object

        TEMPLATE_T
        inline enable_if_not_pointer_not_stl<T> Recv(T &obj, const int src, const int tag, const Comm &comm) {
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            msg.packRootVar(obj);
        };

        TEMPLATE_T_F(TransportRecv)
        inline enable_if_not_pointer_not_stl<T> Recv(T &obj, const int src, const int tag, const Comm &comm) {
            Message<TransportRecv, HASH_MAP> msg(src, tag, comm);
            msg. template packRootVar<T, F>(obj);
        };
        
        TEMPLATE_T
        inline enable_if_deep_not_pointer_not_stl<T> BufferedRecv(T &obj, const int src, const int tag, const Comm &comm) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::Recv(buffer, bufferSize, src, tag, comm);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(obj);

            MEL::MemFree(buffer);
        };

        TEMPLATE_T_F(TransportBufferRead)
        inline enable_if_not_pointer_not_stl<T> BufferedRecv(T &obj, const int src, const int tag, const Comm &comm) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::Recv(buffer, bufferSize, src, tag, comm);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootVar<T, F>(obj);

            MEL::MemFree(buffer);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Bcast
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer / Length

        TEMPLATE_P
        inline enable_if_pointer<P> Bcast(P &ptr, int const &len, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                Message<TransportBcastRoot, HASH_MAP> msg(root, comm);
                int _len = len;
                msg.packRootVar(_len);
                msg.packRootPtr(ptr, _len);
            }
            else {
                Message<TransportBcast, HASH_MAP> msg(root, comm);
                
                int _len = len;
                msg.packRootVar(_len);
                if (len != _len) MEL::Exit(-1, "MEL::Deep::Bcast(ptr, len) const int len provided does not match incomming message size.");
                msg.packRootPtr(ptr, _len);
            }
        };

        TEMPLATE_P_F2(TransportBcastRoot, TransportBcast)
        inline enable_if_pointer<P> Bcast(P &ptr, int const &len, const int root, const Comm &comm) {
            typedef typename std::remove_pointer<P>::type T;
            if (MEL::CommRank(comm) == root) {
                Message<TransportBcastRoot, HASH_MAP> msg(root, comm);
                int _len = len;
                msg.packRootVar(_len);
                msg. template packRootPtr<T, F1>(ptr, _len);
            }
            else {
                Message<TransportBcast, HASH_MAP> msg(root, comm);
                
                int _len = len;
                msg.packRootVar(_len);
                if (len != _len) MEL::Exit(-1, "MEL::Deep::Bcast(ptr, len) const int len provided does not match incomming message size.");
                msg. template packRootPtr<T, F2>(ptr, _len);
            }
        };

        TEMPLATE_P
        inline enable_if_pointer<P> Bcast(P &ptr, int &len, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                Message<TransportBcastRoot, HASH_MAP> msg(root, comm);
                msg.packRootVar(len);
                msg.packRootPtr(ptr, len);
            }
            else {
                Message<TransportBcast, HASH_MAP> msg(root, comm);
                
                msg.packRootVar(len);
                msg.packRootPtr(ptr, len);
            }
        };

        TEMPLATE_P_F2(TransportBcastRoot, TransportBcast)
        inline enable_if_pointer<P> Bcast(P &ptr, int &len, const int root, const Comm &comm) {
            typedef typename std::remove_pointer<P>::type T;
            if (MEL::CommRank(comm) == root) {
                Message<TransportBcastRoot, HASH_MAP> msg(root, comm);
                msg.packRootVar(len);
                msg. template packRootPtr<T, F1>(ptr, len);
            }
            else {
                Message<TransportBcast, HASH_MAP> msg(root, comm);
                msg.packRootVar(len);
                msg. template packRootPtr<T, F2>(ptr, len);
            }
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedBcast(P &ptr, int &len, const int root, const Comm &comm, const int bufferSize) {
            if (MEL::CommRank(comm) == root) {
                char *buffer = MEL::MemAlloc<char>(bufferSize);
                Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
                msg.packRootVar(len); 
                msg.packRootPtr(ptr);

                MEL::Deep::Bcast(buffer, msg.getOffset(), root, comm);
                MEL::MemFree(buffer);
            }
            else {
                int _bufferSize;
                char *buffer = nullptr;
                MEL::Deep::Bcast(buffer, _bufferSize, root, comm);

                Message<TransportBufferRead, HASH_MAP> msg(buffer, _bufferSize);
                msg.packRootVar(len); 
                msg.packRootPtr(ptr);

                MEL::MemFree(buffer);
            }
        };

        TEMPLATE_P_F2(TransportBufferWrite, TransportBufferRead)
        inline enable_if_pointer<P> BufferedBcast(P &ptr, int &len, const int root, const Comm &comm, const int bufferSize) {
            typedef typename std::remove_pointer<P>::type T;
            if (MEL::CommRank(comm) == root) {
                char *buffer = MEL::MemAlloc<char>(bufferSize);
                Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
                msg.packRootVar(len); 
                msg. template packRootPtr<T, F1>(ptr);

                MEL::Deep::Bcast(buffer, msg.getOffset(), root, comm);
                MEL::MemFree(buffer);
            }
            else {
                int _bufferSize;
                char *buffer = nullptr;
                MEL::Deep::Bcast(buffer, _bufferSize, root, comm);

                Message<TransportBufferRead, HASH_MAP> msg(buffer, _bufferSize);
                msg.packRootVar(len); 
                msg. template packRootPtr<T, F2>(ptr);

                MEL::MemFree(buffer);
            }
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedBcast(P &ptr, int &len, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                MEL::Deep::BufferedBcast(ptr, len, root, comm, MEL::Deep::BufferSize(ptr, len));
            }
            else {
                MEL::Deep::BufferedBcast(ptr, len, root, comm, 0);
            }
        };

        TEMPLATE_P_F2(TransportBufferWrite, TransportBufferRead)
        inline enable_if_pointer<P> BufferedBcast(P &ptr, int &len, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                MEL::Deep::BufferedBcast<P, HASH_MAP, F1>(ptr, len, root, comm, MEL::Deep::BufferSize<P, HASH_MAP, F1>(ptr, len));
            }
            else {
                MEL::Deep::BufferedBcast<P, HASH_MAP, F2>(ptr, len, root, comm, 0);
            }
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedBcast(P &ptr, int const &len, const int root, const Comm &comm, const int bufferSize) {
            if (MEL::CommRank(comm) == root) {
                char *buffer = MEL::MemAlloc<char>(bufferSize);
                Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
                msg.packRootVar(len);
                msg.packRootPtr(ptr);

                MEL::Deep::Bcast(buffer, msg.getOffset(), root, comm);
                MEL::MemFree(buffer);
            }
            else {
                int _bufferSize;
                char *buffer = nullptr;
                MEL::Deep::Bcast(buffer, _bufferSize, root, comm);

                Message<TransportBufferRead, HASH_MAP> msg(buffer, _bufferSize);
                int _len = len;
                msg.packRootVar(_len);
                if (len != _len) MEL::Exit(-1, "MEL::Deep::BufferedBcast(ptr, len) const int len provided does not match incomming message size.");
                msg.packRootPtr(ptr);

                MEL::MemFree(buffer);
            }
        };

        TEMPLATE_P_F2(TransportBufferWrite, TransportBufferRead)
        inline enable_if_pointer<P> BufferedBcast(P &ptr, int const &len, const int root, const Comm &comm, const int bufferSize) {
            typedef typename std::remove_pointer<P>::type T;
            if (MEL::CommRank(comm) == root) {
                char *buffer = MEL::MemAlloc<char>(bufferSize);
                Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
                msg.packRootVar(len);
                msg. template packRootPtr<T, F1>(ptr);

                MEL::Deep::Bcast(buffer, msg.getOffset(), root, comm);
                MEL::MemFree(buffer);
            }
            else {
                int _bufferSize;
                char *buffer = nullptr;
                MEL::Deep::Bcast(buffer, _bufferSize, root, comm);

                Message<TransportBufferRead, HASH_MAP> msg(buffer, _bufferSize);
                int _len = len;
                msg.packRootVar(_len);
                if (len != _len) MEL::Exit(-1, "MEL::Deep::BufferedBcast(ptr, len) const int len provided does not match incomming message size.");
                msg. template packRootPtr<T, F2>(ptr);

                MEL::MemFree(buffer);
            }
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedBcast(P &ptr, int const &len, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                MEL::Deep::BufferedBcast(ptr, len, root, comm, MEL::Deep::BufferSize(ptr, len));
            }
            else {
                MEL::Deep::BufferedBcast(ptr, len, root, comm, 0);
            }
        };

        TEMPLATE_P_F2(TransportBufferWrite, TransportBufferRead)
        inline enable_if_pointer<P> BufferedBcast(P &ptr, int const &len, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                MEL::Deep::BufferedBcast<P, HASH_MAP, F1>(ptr, len, root, comm, MEL::Deep::BufferSize<P, HASH_MAP, F1>(ptr, len));
            }
            else {
                MEL::Deep::BufferedBcast<P, HASH_MAP, F2>(ptr, len, root, comm, 0);
            }
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer

        TEMPLATE_P
        inline enable_if_pointer<P> Bcast(P &ptr, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                Message<TransportBcastRoot, HASH_MAP> msg(root, comm);
                msg.packRootPtr(ptr);
            }
            else {
                Message<TransportBcast, HASH_MAP> msg(root, comm);
                msg.packRootPtr(ptr);
            }
        };

        TEMPLATE_P_F2(TransportBcastRoot, TransportBcast)
        inline enable_if_pointer<P> Bcast(P &ptr, const int root, const Comm &comm) {
            typedef typename std::remove_pointer<P>::type T;
            if (MEL::CommRank(comm) == root) {
                Message<TransportBcastRoot, HASH_MAP> msg(root, comm);
                msg. template packRootPtr<T, F1>(ptr);
            }
            else {
                Message<TransportBcast, HASH_MAP> msg(root, comm);
                
                msg. template packRootPtr<T, F2>(ptr);
            }
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedBcast(P &ptr, const int root, const Comm &comm, const int bufferSize) {
            if (MEL::CommRank(comm) == root) {
                char *buffer = MEL::MemAlloc<char>(bufferSize);
                Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
                msg.packRootPtr(ptr);

                MEL::Deep::Bcast(buffer, msg.getOffset(), root, comm);
                MEL::MemFree(buffer);
            }
            else {
                int _bufferSize;
                char *buffer = nullptr;
                MEL::Deep::Bcast(buffer, _bufferSize, root, comm);

                Message<TransportBufferRead, HASH_MAP> msg(buffer, _bufferSize);
                msg.packRootPtr(ptr);

                MEL::MemFree(buffer);
            }
        };

        TEMPLATE_P_F2(TransportBufferWrite, TransportBufferRead)
        inline enable_if_pointer<P> BufferedBcast(P &ptr, const int root, const Comm &comm, const int bufferSize) {
            typedef typename std::remove_pointer<P>::type T;
            if (MEL::CommRank(comm) == root) {
                char *buffer = MEL::MemAlloc<char>(bufferSize);
                Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
                msg. template packRootPtr<T, F1>(ptr);

                MEL::Deep::Bcast(buffer, msg.getOffset(), root, comm);
                MEL::MemFree(buffer);
            }
            else {
                int _bufferSize;
                char *buffer = nullptr;
                MEL::Deep::Bcast(buffer, _bufferSize, root, comm);

                Message<TransportBufferRead, HASH_MAP> msg(buffer, _bufferSize);
                msg. template packRootPtr<T, F2>(ptr);

                MEL::MemFree(buffer);
            }
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedBcast(P &ptr, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                MEL::Deep::BufferedBcast(ptr, root, comm, MEL::Deep::BufferSize(ptr));
            }
            else {
                MEL::Deep::BufferedBcast(ptr, root, comm, 0);
            }
        };

        TEMPLATE_P_F2(TransportBufferWrite, TransportBufferRead)
        inline enable_if_pointer<P> BufferedBcast(P &ptr, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                MEL::Deep::BufferedBcast<P, HASH_MAP, F1>(ptr, root, comm, MEL::Deep::BufferSize<P, HASH_MAP, F1>(ptr));
            }
            else {
                MEL::Deep::BufferedBcast<P, HASH_MAP, F2>(ptr, root, comm, 0);
            }
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // STL

        TEMPLATE_STL
        inline enable_if_stl<S> Bcast(S &obj, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                Message<TransportBcastRoot, HASH_MAP> msg(root, comm);
                msg.packRootSTL(obj);
            }
            else {
                Message<TransportBcast, HASH_MAP> msg(root, comm);
                msg.packRootSTL(obj);
            }
        };

        TEMPLATE_STL_F2(TransportBcastRoot, TransportBcast)
        inline enable_if_stl<S> Bcast(S &obj, const int root, const Comm &comm) {
            typedef typename S::value_type T;
            if (MEL::CommRank(comm) == root) {
                Message<TransportBcastRoot, HASH_MAP> msg(root, comm);
                msg. template packRootSTL<T, F1>(obj);
            }
            else {
                Message<TransportBcast, HASH_MAP> msg(root, comm);
                msg. template packRootSTL<T, F2>(obj);
            }
        };

        TEMPLATE_STL
        inline enable_if_stl<S> BufferedBcast(S &obj, const int root, const Comm &comm, const int bufferSize) {
            if (MEL::CommRank(comm) == root) {
                char *buffer = MEL::MemAlloc<char>(bufferSize);
                Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
                msg.packRootSTL(obj);

                MEL::Deep::Bcast(buffer, msg.getOffset(), root, comm);
                MEL::MemFree(buffer);
            }
            else {
                int _bufferSize;
                char *buffer = nullptr;
                MEL::Deep::Bcast(buffer, _bufferSize, root, comm);

                Message<TransportBufferRead, HASH_MAP> msg(buffer, _bufferSize);
                msg.packRootSTL(obj);

                MEL::MemFree(buffer);
            }
        };

        TEMPLATE_STL_F2(TransportBufferWrite, TransportBufferRead)
        inline enable_if_stl<S> BufferedBcast(S &obj, const int root, const Comm &comm, const int bufferSize) {
            typedef typename S::value_type T;
            if (MEL::CommRank(comm) == root) {
                char *buffer = MEL::MemAlloc<char>(bufferSize);
                Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
                msg. template packRootSTL<T, F1>(obj);

                MEL::Deep::Bcast(buffer, msg.getOffset(), root, comm);
                MEL::MemFree(buffer);
            }
            else {
                int _bufferSize;
                char *buffer = nullptr;
                MEL::Deep::Bcast(buffer, _bufferSize, root, comm);

                Message<TransportBufferRead, HASH_MAP> msg(buffer, _bufferSize);
                msg. template packRootSTL<T, F2>(obj);

                MEL::MemFree(buffer);
            }
        };

        TEMPLATE_STL
        inline enable_if_stl<S> BufferedBcast(S &obj, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                MEL::Deep::BufferedBcast(obj, root, comm, MEL::Deep::BufferSize(obj));
            }
            else {
                MEL::Deep::BufferedBcast(obj, root, comm, 0);
            }
        };

        TEMPLATE_STL_F2(TransportBufferWrite, TransportBufferRead)
        inline enable_if_stl<S> BufferedBcast(S &obj, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                MEL::Deep::BufferedBcast<S, HASH_MAP, F1>(obj, root, comm, MEL::Deep::BufferSize<S, HASH_MAP, F1>(obj));
            }
            else {
                MEL::Deep::BufferedBcast<S, HASH_MAP, F2>(obj, root, comm, 0);
            }
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Object

        TEMPLATE_T
        inline enable_if_not_pointer_not_stl<T> Bcast(T &obj, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                Message<TransportBcastRoot, HASH_MAP> msg(root, comm);
                msg.packRootVar(obj);
            }
            else {
                Message<TransportBcast, HASH_MAP> msg(root, comm);
                msg.packRootVar(obj);
            }
        };

        TEMPLATE_T_F2(TransportBcastRoot, TransportBcast)
        inline enable_if_not_pointer_not_stl<T> Bcast(T &obj, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                Message<TransportBcastRoot, HASH_MAP> msg(root, comm);
                msg. template packRootVar<T, F1>(obj);
            }
            else {
                Message<TransportBcast, HASH_MAP> msg(root, comm);
                msg. template packRootVar<T, F2>(obj);
            }
        };

        TEMPLATE_T
        inline enable_if_deep_not_pointer_not_stl<T> BufferedBcast(T &obj, const int root, const Comm &comm, const int bufferSize) {
            if (MEL::CommRank(comm) == root) {
                char *buffer = MEL::MemAlloc<char>(bufferSize);
                Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
                msg.packRootVar(obj);

                MEL::Deep::Bcast(buffer, msg.getOffset(), root, comm);
                MEL::MemFree(buffer);
            }
            else {
                int _bufferSize;
                char *buffer = nullptr;
                MEL::Deep::Bcast(buffer, _bufferSize, root, comm);

                Message<TransportBufferRead, HASH_MAP> msg(buffer, _bufferSize);
                msg.packRootVar(obj);

                MEL::MemFree(buffer);
            }
        };

        TEMPLATE_T_F2(TransportBufferWrite, TransportBufferRead)
        inline enable_if_not_pointer_not_stl<T> BufferedBcast(T &obj, const int root, const Comm &comm, const int bufferSize) {
            if (MEL::CommRank(comm) == root) {
                char *buffer = MEL::MemAlloc<char>(bufferSize);
                Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
                msg. template packRootVar<T, F1>(obj);

                MEL::Deep::Bcast(buffer, msg.getOffset(), root, comm);
                MEL::MemFree(buffer);
            }
            else {
                int _bufferSize;
                char *buffer = nullptr;
                MEL::Deep::Bcast(buffer, _bufferSize, root, comm);

                Message<TransportBufferRead, HASH_MAP> msg(buffer, _bufferSize);
                msg. template packRootVar<T, F2>(obj);

                MEL::MemFree(buffer);
            }
        };

        TEMPLATE_T
        inline enable_if_deep_not_pointer_not_stl<T> BufferedBcast(T &obj, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                MEL::Deep::BufferedBcast(obj, root, comm, MEL::Deep::BufferSize(obj));
            }
            else {
                MEL::Deep::BufferedBcast(obj, root, comm, 0);
            }
        };

        TEMPLATE_T_F2(TransportBufferWrite, TransportBufferRead)
        inline enable_if_not_pointer_not_stl<T> BufferedBcast(T &obj, const int root, const Comm &comm) {
            if (MEL::CommRank(comm) == root) {
                MEL::Deep::BufferedBcast<T, HASH_MAP, F1>(obj, root, comm, MEL::Deep::BufferSize<T, HASH_MAP, F1>(obj));
            }
            else {
                MEL::Deep::BufferedBcast<T, HASH_MAP, F2>(obj, root, comm, 0);
            }
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // MPI_File Write
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer / Length

        TEMPLATE_P
            inline enable_if_pointer<P> FileWrite(P &ptr, int const &len, MEL::File &file) {
            Message<TransportFileWrite, HASH_MAP> msg(file);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);
        };

        TEMPLATE_P_F(TransportFileWrite)
        inline enable_if_pointer<P> FileWrite(P &ptr, int const &len, MEL::File &file) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportFileWrite, HASH_MAP> msg(file);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, int const &len, MEL::File &file, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferWrite)
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, int const &len, MEL::File &file, const int bufferSize) {
            typedef typename std::remove_pointer<P>::type T;
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, int const &len, MEL::File &file) {
            MEL::Deep::BufferedFileWrite(ptr, len, file, MEL::Deep::BufferSize(ptr, len));
        };

        TEMPLATE_P_F(TransportBufferWrite)
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, int const &len, MEL::File &file) {
            MEL::Deep::BufferedFileWrite<P, HASH_MAP, F>(ptr, len, file, MEL::Deep::BufferSize<P, HASH_MAP, F>(ptr, len));
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer

        TEMPLATE_P
        inline enable_if_pointer<P> FileWrite(P &ptr, MEL::File &file) {
            Message<TransportFileWrite, HASH_MAP> msg(file);
            msg.packRootPtr(ptr);
        };

        TEMPLATE_P_F(TransportFileWrite)
        inline enable_if_pointer<P> FileWrite(P &ptr, MEL::File &file) {
            Message<TransportFileWrite, HASH_MAP> msg(file);
            msg. template packRootPtr<P, F>(ptr);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, MEL::File &file, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootPtr(ptr);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferWrite)
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, MEL::File &file, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootPtr<P, F>(ptr);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, MEL::File &file) {
            MEL::Deep::BufferedFileWrite(ptr, file, MEL::Deep::BufferSize(ptr));
        };

        TEMPLATE_P_F(TransportBufferWrite)
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, MEL::File &file) {
            MEL::Deep::BufferedFileWrite<P, HASH_MAP, F>(ptr, file, MEL::Deep::BufferSize<P, HASH_MAP, F>(ptr));
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // STL

        TEMPLATE_STL
        inline enable_if_stl<S> FileWrite(S &obj, MEL::File &file) {
            Message<TransportFileWrite, HASH_MAP> msg(file);
            msg.packRootSTL(obj);
        };

        TEMPLATE_STL_F(TransportFileWrite)
        inline enable_if_stl<S> FileWrite(S &obj, MEL::File &file) {
            typedef typename S::value_type T;
            Message<TransportFileWrite, HASH_MAP> msg(file);
            msg. template packRootSTL<T, F>(obj);
        };

        TEMPLATE_STL
        inline enable_if_stl<S> BufferedFileWrite(S &obj, MEL::File &file, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootSTL(obj);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_STL_F(TransportBufferWrite)
        inline enable_if_stl<S> BufferedFileWrite(S &obj, MEL::File &file, const int bufferSize) {
            typedef typename S::value_type T;
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootSTL<T, F>(obj);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_STL
        inline enable_if_stl<S> BufferedFileWrite(S &obj, MEL::File &file) {
            MEL::Deep::BufferedFileWrite(obj, file, MEL::Deep::BufferSize(obj));
        };

        TEMPLATE_STL_F(TransportBufferWrite)
        inline enable_if_stl<S> BufferedFileWrite(S &obj, MEL::File &file) {
            MEL::Deep::BufferedFileWrite<S, HASH_MAP, F>(obj, file, MEL::Deep::BufferSize<S, HASH_MAP, F>(obj));
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Object

        TEMPLATE_T
        inline enable_if_not_pointer_not_stl<T> FileWrite(T &obj, MEL::File &file) {
            Message<TransportFileWrite, HASH_MAP> msg(file);
            msg.packRootVar(obj);
        };

        TEMPLATE_T_F(TransportFileWrite)
        inline enable_if_not_pointer_not_stl<T> FileWrite(T &obj, MEL::File &file) {
            Message<TransportFileWrite, HASH_MAP> msg(file);
            msg. template packRootVar<T, F>(obj);
        };

        TEMPLATE_T
        inline enable_if_deep_not_pointer_not_stl<T> BufferedFileWrite(T &obj, MEL::File &file) {
            MEL::Deep::BufferedFileWrite(obj, file, MEL::Deep::BufferSize(obj));
        };

        TEMPLATE_T_F(TransportBufferWrite)
        inline enable_if_not_pointer_not_stl<T> BufferedFileWrite(T &obj, MEL::File &file) {
            MEL::Deep::BufferedFileWrite<T, HASH_MAP, F>(obj, file, MEL::Deep::BufferSize<T, HASH_MAP, F>(obj));
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // MPI_File Read
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer / Length

        TEMPLATE_P
        inline enable_if_pointer<P> FileRead(P &ptr, int const &len, MEL::File &file) {
            Message<TransportFileRead, HASH_MAP> msg(file);
            int _len = len;
            msg.packRootVar(_len);
            if (len != _len) MEL::Exit(-1, "MEL::Deep::FileRead(ptr, len) const int len provided does not match incomming message size.");
            msg.packRootPtr(ptr, _len);
        };

        TEMPLATE_P_F(TransportFileRead)
        inline enable_if_pointer<P> FileRead(P &ptr, int const &len, MEL::File &file) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportFileRead, HASH_MAP> msg(file);
            int _len = len;
            msg.packRootVar(_len);
            if (len != _len) MEL::Exit(-1, "MEL::Deep::FileRead(ptr, len) const int len provided does not match incomming message size.");
            msg. template packRootPtr<T, F>(ptr, _len);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> FileRead(P &ptr, int &len, MEL::File &file) {
            Message<TransportFileRead, HASH_MAP> msg(file);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);
        };

        TEMPLATE_P_F(TransportFileRead)
        inline enable_if_pointer<P> FileRead(P &ptr, int &len, MEL::File &file) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportFileRead, HASH_MAP> msg(file);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileRead(P &ptr, int &len, MEL::File &file) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferRead)
        inline enable_if_pointer<P> BufferedFileRead(P &ptr, int &len, MEL::File &file) {
            typedef typename std::remove_pointer<P>::type T;
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileRead(P &ptr, int const &len, MEL::File &file) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            int _len = len;
            msg.packRootVar(_len);
            if (len != _len) MEL::Exit(-1, "MEL::Deep::BufferedFileRead(ptr, len) const int len provided does not match incomming message size.");
            msg.packRootPtr(ptr, _len);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferRead)
        inline enable_if_pointer<P> BufferedFileRead(P &ptr, int const &len, MEL::File &file) {
            typedef typename std::remove_pointer<P>::type T;
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            int _len = len;
            msg.packRootVar(_len);
            if (len != _len) MEL::Exit(-1, "MEL::Deep::BufferedFileRead(ptr, len) const int len provided does not match incomming message size.");
            msg. template packRootPtr<T, F>(ptr, _len);

            MEL::MemFree(buffer);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer

        TEMPLATE_P
        inline enable_if_pointer<P> FileRead(P &ptr, MEL::File &file) {
            Message<TransportFileRead, HASH_MAP> msg(file);
            msg.packRootPtr(ptr);
        };

        TEMPLATE_P_F(TransportFileRead)
        inline enable_if_pointer<P> FileRead(P &ptr, MEL::File &file) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportFileRead, HASH_MAP> msg(file);
            msg. template packRootPtr<T, F>(ptr);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileRead(P &ptr, MEL::File &file) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootPtr(ptr);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferRead)
        inline enable_if_pointer<P> BufferedFileRead(P &ptr, MEL::File &file) {
            typedef typename std::remove_pointer<P>::type T;
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootPtr<T, F>(ptr);

            MEL::MemFree(buffer);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // STL

        TEMPLATE_STL
        inline enable_if_stl<S> FileRead(S &obj, MEL::File &file) {
            Message<TransportFileRead, HASH_MAP> msg(file);
            msg.packRootSTL(obj);
        };

        TEMPLATE_STL_F(TransportFileRead)
        inline enable_if_stl<S> FileRead(S &obj, MEL::File &file) {
            typedef typename S::value_type T;
            Message<TransportFileRead, HASH_MAP> msg(file);
            msg. template packRootSTL<T, F>(obj);
        };

        TEMPLATE_STL
        inline enable_if_stl<S> BufferedFileRead(S &obj, MEL::File &file) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootSTL(obj);

            MEL::MemFree(buffer);
        };

        TEMPLATE_STL_F(TransportBufferRead)
        inline enable_if_stl<S> BufferedFileRead(S &obj, MEL::File &file) {
            typedef typename S::value_type T;
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootSTL<T, F>(obj);

            MEL::MemFree(buffer);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Object

        TEMPLATE_T
        inline enable_if_not_pointer_not_stl<T> FileRead(T &obj, MEL::File &file) {
            Message<TransportFileRead, HASH_MAP> msg(file);
            msg.packRootVar(obj);
        };

        TEMPLATE_T_F(TransportFileRead)
        inline enable_if_not_pointer_not_stl<T> FileRead(T &obj, MEL::File &file) {
            Message<TransportFileRead, HASH_MAP> msg(file);
            msg. template packRootVar<T, F>(obj);
        };

        TEMPLATE_T
        inline enable_if_deep_not_pointer_not_stl<T> BufferedFileRead(T &obj, MEL::File &file) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(obj);

            MEL::MemFree(buffer);
        };

        TEMPLATE_T_F(TransportBufferRead)
        inline enable_if_not_pointer_not_stl<T> BufferedFileRead(T &obj, MEL::File &file) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootVar<T, F>(obj);

            MEL::MemFree(buffer);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // STL File Write
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer / Length

        TEMPLATE_P
        inline enable_if_pointer<P> FileWrite(P &ptr, int const &len, std::ofstream &file) {
            Message<TransportSTLFileWrite, HASH_MAP> msg(file);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);
        };

        TEMPLATE_P_F(TransportSTLFileWrite)
        inline enable_if_pointer<P> FileWrite(P &ptr, int const &len, std::ofstream &file) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportSTLFileWrite, HASH_MAP> msg(file);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, int const &len, std::ofstream &file, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferWrite)
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, int const &len, std::ofstream &file, const int bufferSize) {
            typedef typename std::remove_pointer<P>::type T;
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, int const &len, std::ofstream &file) {
            MEL::Deep::BufferedFileWrite(ptr, len, file, MEL::Deep::BufferSize(ptr, len));
        };

        TEMPLATE_P_F(TransportBufferWrite)
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, int const &len, std::ofstream &file) {
            MEL::Deep::BufferedFileWrite<P, HASH_MAP, F>(ptr, len, file, MEL::Deep::BufferSize<P, HASH_MAP, F>(ptr, len));
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer

        TEMPLATE_P
        inline enable_if_pointer<P> FileWrite(P &ptr, std::ofstream &file) {
            Message<TransportSTLFileWrite, HASH_MAP> msg(file);
            msg.packRootPtr(ptr);
        };

        TEMPLATE_P_F(TransportSTLFileWrite)
        inline enable_if_pointer<P> FileWrite(P &ptr, std::ofstream &file) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportSTLFileWrite, HASH_MAP> msg(file);
            msg. template packRootPtr<T, F>(ptr);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, std::ofstream &file, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootPtr(ptr);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferWrite)
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, std::ofstream &file, const int bufferSize) {
            typedef typename std::remove_pointer<P>::type T;
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootPtr<T, F>(ptr);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, std::ofstream &file) {
            MEL::Deep::BufferedFileWrite(ptr, file, MEL::Deep::BufferSize(ptr));
        };

        TEMPLATE_P
            inline enable_if_pointer<P> BufferedFileWrite_FUCK(P &ptr, std::ofstream &file) {
            MEL::Deep::BufferedFileWrite(ptr, file, MEL::Deep::BufferSize(ptr));
        };

        TEMPLATE_P_F(TransportBufferWrite)
        inline enable_if_pointer<P> BufferedFileWrite(P &ptr, std::ofstream &file) {
            MEL::Deep::BufferedFileWrite<P, HASH_MAP, F>(ptr, file, MEL::Deep::BufferSize<P, HASH_MAP, F>(ptr));
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // STL 

        TEMPLATE_STL
        inline enable_if_stl<S> FileWrite(S &obj, std::ofstream &file) {
            Message<TransportSTLFileWrite, HASH_MAP> msg(file);
            msg.packRootSTL(obj);
        };

        TEMPLATE_STL_F(TransportSTLFileWrite)
        inline enable_if_stl<S> FileWrite(S &obj, std::ofstream &file) {
            typedef typename S::value_type T;
            Message<TransportSTLFileWrite, HASH_MAP> msg(file);
            msg. template packRootSTL<T, F>(obj);
        };

        TEMPLATE_STL
        inline enable_if_stl<S> BufferedFileWrite(S &obj, std::ofstream &file, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootSTL(obj);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_STL_F(TransportBufferWrite)
        inline enable_if_stl<S> BufferedFileWrite(S &obj, std::ofstream &file, const int bufferSize) {
            typedef typename S::value_type T;
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootSTL<T, F>(obj);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_STL
        inline enable_if_stl<S> BufferedFileWrite(S &obj, std::ofstream &file) {
            MEL::Deep::BufferedFileWrite(obj, file, MEL::Deep::BufferSize(obj));
        };

        TEMPLATE_STL_F(TransportBufferWrite)
        inline enable_if_stl<S> BufferedFileWrite(S &obj, std::ofstream &file) {
            MEL::Deep::BufferedFileWrite<S, HASH_MAP, F>(obj, file, MEL::Deep::BufferSize<S, HASH_MAP, F>(obj));
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Object

        TEMPLATE_T
        inline enable_if_not_pointer_not_stl<T> FileWrite(T &obj, std::ofstream &file) {
            Message<TransportSTLFileWrite, HASH_MAP> msg(file);
            msg.packRootVar(obj);
        };

        TEMPLATE_T_F(TransportSTLFileWrite)
        inline enable_if_not_pointer_not_stl<T> FileWrite(T &obj, std::ofstream &file) {
            Message<TransportSTLFileWrite, HASH_MAP> msg(file);
            msg. template packRootVar<T, F>(obj);
        };

        TEMPLATE_T
        inline enable_if_deep_not_pointer_not_stl<T> BufferedFileWrite(T &obj, std::ofstream &file, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(obj);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_T_F(TransportBufferWrite)
        inline enable_if_not_pointer_not_stl<T> BufferedFileWrite(T &obj, std::ofstream &file, const int bufferSize) {
            char *buffer = MEL::MemAlloc<char>(bufferSize);
            Message<TransportBufferWrite, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootVar<T, F>(obj);

            MEL::Deep::FileWrite(buffer, msg.getOffset(), file);
            MEL::MemFree(buffer);
        };

        TEMPLATE_T
        inline enable_if_deep_not_pointer_not_stl<T> BufferedFileWrite(T &obj, std::ofstream &file) {
            MEL::Deep::BufferedFileWrite(obj, file, MEL::Deep::BufferSize(obj));
        };

        TEMPLATE_T_F(TransportBufferWrite)
        inline enable_if_not_pointer_not_stl<T> BufferedFileWrite(T &obj, std::ofstream &file) {
            MEL::Deep::BufferedFileWrite<T, HASH_MAP, F>(obj, file, MEL::Deep::BufferSize<T, HASH_MAP, F>(obj));
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // STL File Read
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer / Length

        TEMPLATE_P
        inline enable_if_pointer<P> FileRead(P &ptr, int const &len, std::ifstream &file) {
            Message<TransportSTLFileRead, HASH_MAP> msg(file);
            int _len = len;
            msg.packRootVar(_len);
            if (len != _len) MEL::Exit(-1, "MEL::Deep::FileRead(ptr, len) const int len provided does not match incomming message size.");
            msg.packRootPtr(ptr, _len);
        };

        TEMPLATE_P_F(TransportSTLFileRead)
        inline enable_if_pointer<P> FileRead(P &ptr, int const &len, std::ifstream &file) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportSTLFileRead, HASH_MAP> msg(file);
            int _len = len;
            msg.packRootVar(_len);
            if (len != _len) MEL::Exit(-1, "MEL::Deep::FileRead(ptr, len) const int len provided does not match incomming message size.");
            msg. template packRootPtr<T, F>(ptr, _len);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> FileRead(P &ptr, int &len, std::ifstream &file) {
            Message<TransportSTLFileRead, HASH_MAP> msg(file);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);
        };

        TEMPLATE_P_F(TransportSTLFileRead)
        inline enable_if_pointer<P> FileRead(P &ptr, int &len, std::ifstream &file) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportSTLFileRead, HASH_MAP> msg(file);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileRead(P &ptr, int &len, std::ifstream &file) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(len);
            msg.packRootPtr(ptr, len);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferRead)
        inline enable_if_pointer<P> BufferedFileRead(P &ptr, int &len, std::ifstream &file) {
            typedef typename std::remove_pointer<P>::type T;
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(len);
            msg. template packRootPtr<T, F>(ptr, len);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileRead(P &ptr, const int len, std::ifstream &file) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            int _len = len;
            msg.packRootVar(_len);
            if (len != _len) MEL::Exit(-1, "MEL::Deep::BufferedFileRead(ptr, len) const int len provided does not match incomming message size.");
            msg.packRootPtr(ptr, _len);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferRead)
        inline enable_if_pointer<P> BufferedFileRead(P &ptr, const int len, std::ifstream &file) {
            typedef typename std::remove_pointer<P>::type T;
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            int _len = len;
            msg.packRootVar(_len);
            if (len != _len) MEL::Exit(-1, "MEL::Deep::BufferedFileRead(ptr, len) const int len provided does not match incomming message size.");
            msg. template packRootPtr<T, F>(ptr, _len);

            MEL::MemFree(buffer);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointer

        TEMPLATE_P
        inline enable_if_pointer<P> FileRead(P &ptr, std::ifstream &file) {
            Message<TransportSTLFileRead, HASH_MAP> msg(file);
            msg.packRootPtr(ptr);
        };

        TEMPLATE_P_F(TransportSTLFileRead)
        inline enable_if_pointer<P> FileRead(P &ptr, std::ifstream &file) {
            typedef typename std::remove_pointer<P>::type T;
            Message<TransportSTLFileRead, HASH_MAP> msg(file);
            msg. template packRootPtr<T, F>(ptr);
        };

        TEMPLATE_P
        inline enable_if_pointer<P> BufferedFileRead(P &ptr, std::ifstream &file) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootPtr(ptr);

            MEL::MemFree(buffer);
        };

        TEMPLATE_P_F(TransportBufferRead)
        inline enable_if_pointer<P> BufferedFileRead(P &ptr, std::ifstream &file) {
            typedef typename std::remove_pointer<P>::type T;
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootPtr<T, F>(ptr);

            MEL::MemFree(buffer);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // STL

        TEMPLATE_STL
        inline enable_if_stl<S> FileRead(S &obj, std::ifstream &file) {
            Message<TransportSTLFileRead, HASH_MAP> msg(file);
            msg.packRootSTL(obj);
        };

        TEMPLATE_STL_F(TransportSTLFileRead)
        inline enable_if_stl<S> FileRead(S &obj, std::ifstream &file) {
            typedef typename S::value_type T;
            Message<TransportSTLFileRead, HASH_MAP> msg(file);
            msg. template packRootSTL<T, F>(obj);
        };

        TEMPLATE_STL
        inline enable_if_stl<S> BufferedFileRead(S &obj, std::ifstream &file) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootSTL(obj);
            
            MEL::MemFree(buffer);
        };

        TEMPLATE_STL_F(TransportBufferRead)
        inline enable_if_stl<S> BufferedFileRead(S &obj, std::ifstream &file) {
            typedef typename S::value_type T;
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootSTL<T, F>(obj);

            MEL::MemFree(buffer);
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Object

        TEMPLATE_T
        inline enable_if_not_pointer_not_stl<T> FileRead(T &obj, std::ifstream &file) {
            Message<TransportSTLFileRead, HASH_MAP> msg(file);
            msg.packRootVar(obj);
        };

        TEMPLATE_T_F(TransportSTLFileRead)
        inline enable_if_not_pointer_not_stl<T> FileRead(T &obj, std::ifstream &file) {
            Message<TransportSTLFileRead, HASH_MAP> msg(file);
            msg. template packRootVar<T, F>(obj);
        };

        TEMPLATE_T
        inline enable_if_deep_not_pointer_not_stl<T> BufferedFileRead(T &obj, std::ifstream &file) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg.packRootVar(obj);

            MEL::MemFree(buffer);
        };

        TEMPLATE_T_F(TransportBufferRead)
        inline enable_if_not_pointer_not_stl<T> BufferedFileRead(T &obj, std::ifstream &file) {
            int bufferSize;
            char *buffer = nullptr;
            MEL::Deep::FileRead(buffer, bufferSize, file);

            Message<TransportBufferRead, HASH_MAP> msg(buffer, bufferSize);
            msg. template packRootVar<T, F>(obj);

            MEL::MemFree(buffer);
        };

#undef TEMPLATE_STL
#undef TEMPLATE_T
#undef TEMPLATE_P

#undef TEMPLATE_STL_F
#undef TEMPLATE_T_F
#undef TEMPLATE_P_F

#undef TEMPLATE_STL_F2
#undef TEMPLATE_T_F2
#undef TEMPLATE_P_F2
    };
};
