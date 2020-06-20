// Copyright (C) 2020 Yixuan Qiu <yixuan.qiu@cos.name>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SPECTRA_SYM_GEIGS_SHIFT_SOLVER_H
#define SPECTRA_SYM_GEIGS_SHIFT_SOLVER_H

#include "SymEigsBase.h"
#include "Util/GEigsMode.h"
#include "MatOp/internal/SymGEigsShiftInvertOp.h"

namespace Spectra {

///
/// \ingroup GEigenSolver
///
/// This class implements the generalized eigen solver for real symmetric
/// matrices, i.e., to solve \f$Ax=\lambda Bx\f$ where \f$A\f$ and \f$B\f$ are symmetric
/// matrices. A spectral transform is applied to seek interior
/// generalized eigenvalues with respect to some shift \f$\sigma\f$.
///
/// There are different modes of this solver, specified by the template parameter `Mode`.
/// See the pages for the specialized classes for details.
/// - The shift-and-invert mode transforms the problem into \f$(A-\sigma B)^{-1}Bx=\nu x\f$,
///   where \f$\nu=1/(\lambda-\sigma)\f$. This mode assumes that \f$B\f$ is positive definite.
///   See \ref SymGEigsShiftSolver<Scalar, OpType, BOpType, GEigsMode::ShiftInvert>
///   "SymGEigsShiftSolver (Shift-and-invert mode)" for more details.

// Empty class template
template <typename Scalar,
          typename OpType,
          typename BOpType,
          GEigsMode Mode>
class SymGEigsShiftSolver
{};

///
/// \ingroup GEigenSolver
///
/// This class implements the generalized eigen solver for real symmetric
/// matrices using the shift-and-invert spectral transformation. The original problem is
/// to solve \f$Ax=\lambda Bx\f$, where \f$A\f$ is symmetric and \f$B\f$ is positive definite.
/// The transformed problem is \f$(A-\sigma B)^{-1}Bx=\nu x\f$, where
/// \f$\nu=1/(\lambda-\sigma)\f$, and \f$\sigma\f$ is a user-specified shift.
///
/// This solver requires two matrix operation objects: one to compute \f$y=(A-\sigma B)^{-1}x\f$
/// for any vector \f$v\f$, and one for the matrix multiplication \f$Bv\f$.
///
/// If \f$A\f$ and \f$B\f$ are stored as Eigen matrices, then the first operation object
/// can be created using the SymShiftInvert class, and the second one can be created
/// using the DenseSymMatProd or SparseSymMatProd classes. If the users need to define their
/// own operation classes, then they should implement all the public member functions as
/// in those built-in classes.
///
/// \tparam Scalar   The element type of the matrix.
///                  Currently supported types are `float`, `double`, and `long double`.
/// \tparam OpType   The type of the first operation object. Users could either
///                  use the wrapper class SymShiftInvert, or define their own that implements
///                  all the public member functions as in SymShiftInvert.
/// \tparam BOpType  The name of the matrix operation class for \f$B\f$. Users could either
///                  use the wrapper classes such as DenseSymMatProd and
///                  SparseSymMatProd, or define their own that implements all the
///                  public member functions as in DenseSymMatProd.
/// \tparam Mode     Mode of the generalized eigen solver. In this solver
///                  it is Spectra::GEigsMode::ShiftInvert.
///
/// Below is an example that demonstrates the usage of this class.
///
/// \code{.cpp}
/// #include <Eigen/Core>
/// #include <Eigen/SparseCore>
/// #include <Spectra/SymGEigsShiftSolver.h>
/// #include <Spectra/MatOp/SymShiftInvert.h>
/// #include <Spectra/MatOp/SparseSymMatProd.h>
/// #include <iostream>
///
/// using namespace Spectra;
///
/// int main()
/// {
///     // We are going to solve the generalized eigenvalue problem A * x = lambda * B * x
///     const int n = 100;
///
///     // Define the A matrix
///     Eigen::MatrixXd M = Eigen::MatrixXd::Random(n, n);
///     Eigen::MatrixXd A = M + M.transpose();
///
///     // Define the B matrix, a band matrix with 2 on the diagonal and 1 on the subdiagonals
///     Eigen::SparseMatrix<double> B(n, n);
///     B.reserve(Eigen::VectorXi::Constant(n, 3));
///     for (int i = 0; i < n; i++)
///     {
///         B.insert(i, i) = 2.0;
///         if (i > 0)
///             B.insert(i - 1, i) = 1.0;
///         if (i < n - 1)
///             B.insert(i + 1, i) = 1.0;
///     }
///
///     // Construct matrix operation objects using the wrapper classes
///     // A is dense, B is sparse
///     using OpType = SymShiftInvert<double, Eigen::Dense, Eigen::Sparse>;
///     using BOpType = SparseSymMatProd<double>;
///     OpType op(A, B);
///     BOpType Bop(B);
///
///     // Construct generalized eigen solver object, seeking three generalized eigenvalues
///     // that are closest to zero. This is equivalent to specifying a shift sigma = 0.0
///     // combined with the SortRule::LargestMagn selection rule
///     SymGEigsShiftSolver<double, OpType, BOpType, GEigsMode::ShiftInvert>
///         geigs(op, Bop, 3, 6, 0.0);
///
///     // Initialize and compute
///     geigs.init();
///     int nconv = geigs.compute(SortRule::LargestMagn);
///
///     // Retrieve results
///     Eigen::VectorXd evalues;
///     Eigen::MatrixXd evecs;
///     if (geigs.info() == CompInfo::Successful)
///     {
///         evalues = geigs.eigenvalues();
///         evecs = geigs.eigenvectors();
///     }
///
///     std::cout << "Number of converged generalized eigenvalues: " << nconv << std::endl;
///     std::cout << "Generalized eigenvalues found:\n" << evalues << std::endl;
///     std::cout << "Generalized eigenvectors found:\n" << evecs.topRows(10) << std::endl;
///
///     return 0;
/// }
/// \endcode

// Partial specialization for mode = GEigsMode::ShiftInvert
template <typename Scalar,
          typename OpType,
          typename BOpType>
class SymGEigsShiftSolver<Scalar, OpType, BOpType, GEigsMode::ShiftInvert> :
    public SymEigsBase<Scalar, SymGEigsShiftInvertOp<Scalar, OpType, BOpType>, BOpType>
{
private:
    using Index = Eigen::Index;
    using Array = Eigen::Array<Scalar, Eigen::Dynamic, 1>;

    using Base = SymEigsBase<Scalar, SymGEigsShiftInvertOp<Scalar, OpType, BOpType>, BOpType>;
    using Base::m_nev;
    using Base::m_ritz_val;

    const Scalar m_sigma;

    // First transform back the Ritz values, and then sort
    void sort_ritzpair(SortRule sort_rule) override
    {
        // The eigenvalues we get from the iteration is nu = 1 / (lambda - sigma)
        // So the eigenvalues of the original problem is lambda = 1 / nu + sigma
        m_ritz_val.head(m_nev).array() = Scalar(1) / m_ritz_val.head(m_nev).array() + m_sigma;
        Base::sort_ritzpair(sort_rule);
    }

public:
    ///
    /// Constructor to create a solver object.
    ///
    /// \param op   The matrix operation object that computes \f$y=(A-\sigma B)^{-1}v\f$
    ///             for any vector \f$v\f$. Users could either create the object from the
    ///             wrapper class SymShiftInvert, or define their own that implements all
    ///             the public member functions as in SymShiftInvert.
    /// \param Bop  The \f$B\f$ matrix operation object that implements the matrix-vector
    ///             multiplication \f$Bv\f$. Users could either create the object from the
    ///             wrapper classes such as DenseSymMatProd and SparseSymMatProd, or
    ///             define their own that implements all the public member functions
    ///             as in DenseSymMatProd.
    /// \param nev  Number of eigenvalues requested. This should satisfy \f$1\le nev \le n-1\f$,
    ///             where \f$n\f$ is the size of matrix.
    /// \param ncv  Parameter that controls the convergence speed of the algorithm.
    ///             Typically a larger `ncv` means faster convergence, but it may
    ///             also result in greater memory use and more matrix operations
    ///             in each iteration. This parameter must satisfy \f$nev < ncv \le n\f$,
    ///             and is advised to take \f$ncv \ge 2\cdot nev\f$.
    ///
    SymGEigsShiftSolver(OpType& op, BOpType& Bop, Index nev, Index ncv, const Scalar& sigma) :
        Base(SymGEigsShiftInvertOp<Scalar, OpType, BOpType>(op, Bop), Bop, nev, ncv),
        m_sigma(sigma)
    {
        op.set_shift(m_sigma);
    }
};

}  // namespace Spectra

#endif  // SPECTRA_SYM_GEIGS_SHIFT_SOLVER_H