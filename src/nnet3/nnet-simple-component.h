// nnet3/nnet-simple-component.h

// Copyright 2011-2013  Karel Vesely
//           2012-2017  Johns Hopkins University (author: Daniel Povey)
//                2013  Xiaohui Zhang
//           2014-2016  Vijayaditya Peddinti
//           2014-2015  Guoguo Chen
//                2015  Daniel Galvez
//                2015  Tom Ko

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.

#ifndef KALDI_NNET3_NNET_SIMPLE_COMPONENT_H_
#define KALDI_NNET3_NNET_SIMPLE_COMPONENT_H_

#include "nnet3/nnet-common.h"
#include "nnet3/nnet-component-itf.h"
#include "nnet3/natural-gradient-online.h"
#include <iostream>

namespace kaldi {
namespace nnet3 {

/// @file  nnet-simple-component.h
///   This file contains declarations of components that are "simple", meaning
///   they don't care about the indexes they are operating on, produce one
///   output for one input, and return the kSimpleComponent flag in their
///   Properties(): for example, tanh and affine components.  In
///   nnet-general-component.h there are components that don't fit this pattern.
///
///   Some components that do provide the kSimpleComponent flag are not declared
///   here: see also nnet-normalize-component.h and nnet-combined-component.h

// This "nnet3" version of the p-norm component only supports the 2-norm.
class PnormComponent: public Component {
 public:
  void Init(int32 input_dim, int32 output_dim);
  explicit PnormComponent(int32 input_dim, int32 output_dim) {
    Init(input_dim, output_dim);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kBackpropNeedsOutput;
  }
  PnormComponent(): input_dim_(0), output_dim_(0) { }
  virtual std::string Type() const { return "PnormComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new PnormComponent(input_dim_,
                                                              output_dim_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

 protected:
  int32 input_dim_;
  int32 output_dim_;
};

// This component randomly zeros dropout_proportion of the input
// and the derivatives are backpropagated through the nonzero inputs.
// Typically this component used during training but not in test time.
// The idea is described under the name Dropout, in the paper
// "Dropout: A Simple Way to Prevent Neural Networks from Overfitting".
class DropoutComponent : public RandomComponent {
 public:
  void Init(int32 dim, BaseFloat dropout_proportion = 0.0,
            bool dropout_per_frame = false);

  DropoutComponent(int32 dim, BaseFloat dropout = 0.0,
                   bool dropout_per_frame = false) {
    Init(dim, dropout, dropout_per_frame);
  }

  DropoutComponent(): dim_(0), dropout_proportion_(0.0),
                      dropout_per_frame_(false) { }

  DropoutComponent(const DropoutComponent &other);

  virtual int32 Properties() const {
    return kBackpropInPlace|kSimpleComponent|kBackpropNeedsInput|
        kBackpropNeedsOutput|kRandomComponent;
  }
  virtual std::string Type() const { return "DropoutComponent"; }

  virtual void InitFromConfig(ConfigLine *cfl);

  virtual int32 InputDim() const { return dim_; }

  virtual int32 OutputDim() const { return dim_; }

  virtual void Read(std::istream &is, bool binary);

  // Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual Component* Copy() const;

  virtual std::string Info() const;

  void SetDropoutProportion(BaseFloat dropout_proportion) {
    dropout_proportion_ = dropout_proportion;
  }

  BaseFloat DropoutProportion() const { return dropout_proportion_; }
 private:
  int32 dim_;
  /// dropout-proportion is the proportion that is dropped out,
  /// e.g. if 0.1, we set 10% to zero value.
  BaseFloat dropout_proportion_;
  bool dropout_per_frame_;
};

class ElementwiseProductComponent: public Component {
 public:
  void Init(int32 input_dim, int32 output_dim);
  explicit ElementwiseProductComponent(int32 input_dim, int32 output_dim) {
    Init(input_dim, output_dim);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput;
  }
  ElementwiseProductComponent(): input_dim_(0), output_dim_(0) { }
  virtual std::string Type() const { return "ElementwiseProductComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new ElementwiseProductComponent(input_dim_,
                                                              output_dim_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

 protected:
  int32 input_dim_;
  int32 output_dim_;
};

/*
   Implements the sigmoid nonlinearity, i.e. the function y = exp(-x).

   Configuration values accepted:
      dim              Dimension of this component, e.g. 1024

   Configuration values inherited from NonlinearComponent, and their
   local meanings:
      self-repair-lower-threshold e.g. self-repair-lower-threshold=0.05.  This
                    controls the self-repair mechanism, which for sigmoid units
                    consists of identifying units which are oversaturated (i.e.
                    usually close to -1 or +1) and nudging the inputs to be
                    closer to zero.  It gates on the average derivative of the
                    nonlinearity, which for sigmoid is a value between 0 and
                    0.25.  For units where the average function-derivative
                    accumulated during this iteration (job) of training is less
                    than this threshold, we activate self-repair, which consists
                    of adding (-self-repair-scale * (2*the output of the
                    nonlinearity - 1.0)) to the backpropagated derivatives.
                    This just happens to be a convenient-to-compute function
                    that's +1 for large negative inputs, and -1 for large positive
                    inputs, and smooth in between.
                    The default value of this is -1000, which the code internally
                    maps to 0.05 which is suitable for sigmoid units; if you do set it,
                    you can set it to a value like 0.025 or 0.075.
      self-repair-scale  Scale for the self-repair mechanism; see comments above.
                    default=0, but we usually set this to 1.0e-05 (or
                    occasionally 1.0e-04) in the scripts.

 */
class SigmoidComponent: public NonlinearComponent {
 public:
  explicit SigmoidComponent(const SigmoidComponent &other): NonlinearComponent(other) { }
  SigmoidComponent() { }
  virtual std::string Type() const { return "SigmoidComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsOutput|kPropagateInPlace|kStoresStats;
  }
  virtual Component* Copy() const { return new SigmoidComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual void StoreStats(const CuMatrixBase<BaseFloat> &in_value,
                          const CuMatrixBase<BaseFloat> &out_value,
                          void *memo);
 private:
  // this function is called from Backprop code and only does something if the
  // self-repair-scale config value is set.
  void RepairGradients(const CuMatrixBase<BaseFloat> &out_value,
                       CuMatrixBase<BaseFloat> *in_deriv,
                       SigmoidComponent *to_update) const;

  SigmoidComponent &operator = (const SigmoidComponent &other); // Disallow.
};

/*
   Implements the tanh nonlinearity, i.e. the function y = tanh(x).

   Configuration values accepted:
      dim           Dimension of this component, e.g. 1024

   Configuration values inherited from NonlinearComponent, and their
   local meanings:
      self-repair-lower-threshold e.g. self-repair-lower-threshold=0.2.  This
                    controls the self-repair mechanism, which for tanh units
                    consists of identifying units which are oversaturated (i.e.
                    usually close to -1 or +1) and nudging the inputs to be
                    closer to zero.  It gates on the average derivative of
                    the nonlinearity, which for tanh is a value between 0 and 1.
                    For units where the average function-derivative accumulated
                    during this iteration (job) of training is less than
                    this threshold, we activate self-repair, which consists of
                    adding (-self-repair-scale * the output of the nonlinearity),
                    i.e. (-self-repair-scale * tanh(x)) to the backpropagated
                    derivatives.
                    The default value of this is -1000, which the code internally
                    maps to 0.2 which is suitable for tanh units; if you do set it,
                    you can set it to a value like 0.1 or 0.3.
      self-repair-scale  Scale for the self-repair mechanism; see comments above.
                    default=0, but we usually set this to 1.0e-05 (or
                    occasionally 1.0e-04) in the scripts.
 */
class TanhComponent: public NonlinearComponent {
 public:
  explicit TanhComponent(const TanhComponent &other): NonlinearComponent(other) { }
  TanhComponent() { }
  virtual std::string Type() const { return "TanhComponent"; }
  virtual Component* Copy() const { return new TanhComponent(*this); }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsOutput|kPropagateInPlace|kStoresStats;
  }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual void StoreStats(const CuMatrixBase<BaseFloat> &in_value,
                          const CuMatrixBase<BaseFloat> &out_value,
                          void *memo);
 private:
  // this function is called from Backprop code and only does something if the
  // self-repair-scale config value is set.
  void RepairGradients(const CuMatrixBase<BaseFloat> &out_value,
                       CuMatrixBase<BaseFloat> *in_deriv,
                       TanhComponent *to_update) const;

  TanhComponent &operator = (const TanhComponent &other); // Disallow.
};


/*
   Implements the Rectified Linear Unit nonlinearity, a.k.a. ReLU.

   Configuration values accepted:
      dim              Dimension of this component, e.g. 1024

   Configuration values inherited from NonlinearComponent, and their
   local meanings:
      self-repair-lower-threshold e.g. self-repair-lower-threshold=0.05.  (Lower
                       threshold for self-repair, if set; in this case acts on
                       the average function-derivative, which is the proportion
                       of the time the output is > 0.  For any unit where the
                       average function-derivative is lower than this threshold,
                       we add 'self-repair-scale' to the backpropagated
                       derivatives in backprop.  There is no default
                       (default=-1000, which is interpreted specially).
      self-repair-upper-threshold e.g. self-repair-upper-threshold=0.95.
                       Like self-repair-lower-threshold, but controls self-repair
                       for units that are active *too* much of the time.  Units
                       whose average function-derivative exceeds this threshold
                       will have the negative of 'self-repair-scale' added to their
                       input derivatives in backprop.  There is no default
                       (default=-1000, which is interpreted specially).
      self-repair-scale  Scale for the self-repair mechanism; see comments for
                       self-repair-lower-threshold and self-repair-upper-threshold
                       for details.  default=0, but we usually set this to 1.0e-05
                       (or occasionally 1.0e-04) in the scripts.
 */
class RectifiedLinearComponent: public NonlinearComponent {
 public:
  explicit RectifiedLinearComponent(const RectifiedLinearComponent &other):
      NonlinearComponent(other) { }
  RectifiedLinearComponent() { }
  virtual std::string Type() const { return "RectifiedLinearComponent"; }
  virtual Component* Copy() const { return new RectifiedLinearComponent(*this); }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsOutput|kPropagateInPlace|
        kStoresStats|(block_dim_ != dim_ ? kInputContiguous : 0);
  }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual void StoreStats(const CuMatrixBase<BaseFloat> &in_value,
                          const CuMatrixBase<BaseFloat> &out_value,
                          void *memo);
 private:
  // this function is called from Backprop code and only does something if the
  // self-repair-scale config value is set.
  void RepairGradients(CuMatrixBase<BaseFloat> *in_deriv,
                       RectifiedLinearComponent *to_update) const;

  RectifiedLinearComponent &operator = (const RectifiedLinearComponent &other); // Disallow.
};


class FixedAffineComponent;
class FixedScaleComponent;
class PerElementScaleComponent;
class PerElementOffsetComponent;

/*
  Affine means a linear function plus an offset.
  Note: although this class can be instantiated, it also
  functions as a base-class for more specialized versions of
  AffineComponent.

  Parameters accepted on the config line, with default if applicable:

     matrix   If specified, a filename containing the parameters of the class as
              a single matrix containing the linear_params, plus the bias_params
              as the last column

     input-dim  The input dimension of the component
     output-dim  The output dimension of the component
     param-stddev=1/sqrt(input-dim)  The standard deviation of the elements of the linear parameters
                      (they will have a Gaussian distribution with this standard deviation).
     bias-stddev=1.0   The standard deviation of the elements of the bias parameters

     orthonormal-constraint=0.0   Can be used to constrain the linear parameter matrix
                       to be semi-orthogonal, see ConstraintOrhonormal() in nnet-utils.h,
                       and http://www.danielpovey.com/files/2018_interspeech_tdnnf.pdf.
*/
class AffineComponent: public UpdatableComponent {
 public:
  virtual int32 InputDim() const { return linear_params_.NumCols(); }
  virtual int32 OutputDim() const { return linear_params_.NumRows(); }

  BaseFloat OrthonormalConstraint() const { return orthonormal_constraint_; }

  virtual std::string Info() const;
  virtual void InitFromConfig(ConfigLine *cfl);

  AffineComponent(): orthonormal_constraint_(0.0) { } // use Init to really initialize.
  virtual std::string Type() const { return "AffineComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kUpdatableComponent|
        kBackpropNeedsInput|kBackpropAdds;
  }


  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

  virtual Component* Copy() const;


  // Some functions from base-class UpdatableComponent.
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const;
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);

  // Some functions that are specific to this class.

  virtual void SetParams(const CuVectorBase<BaseFloat> &bias,
                         const CuMatrixBase<BaseFloat> &linear);
  const CuVector<BaseFloat> &BiasParams() const { return bias_params_; }
  CuVector<BaseFloat> &BiasParams() { return bias_params_; }
  const CuMatrix<BaseFloat> &LinearParams() const { return linear_params_; }
  CuMatrix<BaseFloat> &LinearParams() { return linear_params_; }
  explicit AffineComponent(const AffineComponent &other);
  // The next constructor is used in converting from nnet1.
  AffineComponent(const CuMatrixBase<BaseFloat> &linear_params,
                  const CuVectorBase<BaseFloat> &bias_params,
                  BaseFloat learning_rate);
  // This function resizes the dimensions of the component, setting the
  // parameters to zero, while leaving any other configuration values the same.
  virtual void Resize(int32 input_dim, int32 output_dim);

  void Init(int32 input_dim, int32 output_dim,
            BaseFloat param_stddev, BaseFloat bias_stddev);
 protected:
  void Init(std::string matrix_filename);

  friend class NaturalGradientAffineComponent;
  // This function Update() is for extensibility; child classes may override
  // this, e.g. for natural gradient update.
  virtual void Update(
      const std::string &debug_info,
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv) {
    UpdateSimple(in_value, out_deriv);
  }
  // UpdateSimple is used when *this is a gradient.  Child classes may override
  // this if needed, but typically won't need to.
  virtual void UpdateSimple(
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv);

  const AffineComponent &operator = (const AffineComponent &other); // Disallow.
  CuMatrix<BaseFloat> linear_params_;
  CuVector<BaseFloat> bias_params_;
  // see documentation at the top of this class for more information on the
  // following.
  BaseFloat orthonormal_constraint_;
};

class RepeatedAffineComponent;

/// This class implements an affine transform using a block diagonal matrix
/// e.g., one whose weight matrix is all zeros except for blocks on the
/// diagonal. All these blocks have the same dimensions.
///  input-dim: num cols of block diagonal matrix.
///  output-dim: num rows of block diagonal matrix.
/// num-blocks: number of blocks in diagonal of the matrix.
/// num-blocks must divide both input-dim and output-dim
class BlockAffineComponent : public UpdatableComponent {
 public:
  virtual int32 InputDim() const { return linear_params_.NumCols() * num_blocks_; }
  virtual int32 OutputDim() const { return linear_params_.NumRows(); }

  virtual std::string Info() const;
  virtual void InitFromConfig(ConfigLine *cfl);

  BlockAffineComponent() { }
  virtual std::string Type() const { return "BlockAffineComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kUpdatableComponent|
      kBackpropNeedsInput|kBackpropAdds;
  }

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;

  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

  virtual Component* Copy() const;

  // Functions from base-class UpdatableComponent.
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const;
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);

  explicit BlockAffineComponent(const BlockAffineComponent &other);
  explicit BlockAffineComponent(const RepeatedAffineComponent &rac);
 protected:
  // The matrix linear_params_ has a block structure, with num_blocks_ blocks of
  // equal size.  The blocks are stored in linear_params_ as
  // [ M
  //   N
  //   O ] but we actually treat it as the matrix:
  // [ M 0 0
  //   0 N 0
  //   0 0 O ]
  CuMatrix<BaseFloat> linear_params_;
  CuVector<BaseFloat> bias_params_;
  int32 num_blocks_;
 private:
  // BlockAffine-specific functions.
  void Init(int32 input_dim, int32 output_dim, int32 num_blocks,
            BaseFloat param_stddev, BaseFloat bias_mean,
            BaseFloat bias_stddev);

  const BlockAffineComponent &operator = (const BlockAffineComponent &other); // Disallow.
};

class RepeatedAffineComponent: public UpdatableComponent {
 public:

  virtual int32 InputDim() const { return linear_params_.NumCols() * num_repeats_; }
  virtual int32 OutputDim() const { return linear_params_.NumRows() * num_repeats_; }

  virtual std::string Info() const;
  virtual void InitFromConfig(ConfigLine *cfl);

  RepeatedAffineComponent() { } // use Init to really initialize.
  virtual std::string Type() const { return "RepeatedAffineComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kUpdatableComponent|kBackpropNeedsInput|
        kBackpropAdds|kInputContiguous|kOutputContiguous;
  }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

  virtual Component* Copy() const;

  // Some functions from base-class UpdatableComponent.
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const;
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);

  // Some functions that are specific to this class.
  const CuVector<BaseFloat> &BiasParams() const { return bias_params_; }
  const CuMatrix<BaseFloat> &LinearParams() const { return linear_params_; }
  explicit RepeatedAffineComponent(const RepeatedAffineComponent &other);

  friend BlockAffineComponent::BlockAffineComponent(const RepeatedAffineComponent &rac);
 protected:
  void Init(int32 input_dim, int32 output_dim, int32 num_repeats,
            BaseFloat param_stddev, BaseFloat bias_mean,
            BaseFloat bias_stddev);

  // This function Update(), called from backprop, is broken out for
  // extensibility to natural gradient update.
  virtual void Update(
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv);

  // This function does nothing here but is redefined in child-class
  // NaturalGradientRepeatedAffineComponent.  This help avoid repeated code.
  virtual void SetNaturalGradientConfigs() { }

  const RepeatedAffineComponent &operator = (const RepeatedAffineComponent &other); // Disallow.
  CuMatrix<BaseFloat> linear_params_;
  CuVector<BaseFloat> bias_params_;
  int32 num_repeats_;
};

class NaturalGradientRepeatedAffineComponent: public RepeatedAffineComponent {
 public:
  // Use Init() to really initialize.
  NaturalGradientRepeatedAffineComponent() { }

  // Most of the public functions are inherited from RepeatedAffineComponent.
  virtual std::string Type() const {
    return "NaturalGradientRepeatedAffineComponent";
  }

  virtual Component* Copy() const;

  // Copy constructor
  explicit NaturalGradientRepeatedAffineComponent(
      const NaturalGradientRepeatedAffineComponent &other);

  virtual void ConsolidateMemory();

 private:
  virtual void Update(
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv);

  const NaturalGradientRepeatedAffineComponent &operator=(
      const NaturalGradientRepeatedAffineComponent &other); // Disallow.

  // Applies the default configuration to preconditioner_in_.
  virtual void SetNaturalGradientConfigs();

  // For efficiency reasons we only apply the natural gradient to the input
  // side, i.e. not to the space of output derivatives-- we believe the input
  // side is the more important side.  We don't make the natural-gradient
  // configurable; we just give it a reasonable configuration.
  // Instead of using the individual data-points, for efficiency reasons we use
  // the distribution of per-minibatch summed derivatives over each dimension of
  // the output space, as the source for the Fisher matrix.
  OnlineNaturalGradient preconditioner_in_;
};

class SoftmaxComponent: public NonlinearComponent {
 public:
  explicit SoftmaxComponent(const SoftmaxComponent &other):
      NonlinearComponent(other) { }
  SoftmaxComponent() { }
  virtual std::string Type() const { return "SoftmaxComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateInPlace|kBackpropInPlace|
        kBackpropNeedsOutput|kStoresStats;
  }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual void StoreStats(const CuMatrixBase<BaseFloat> &in_value,
                          const CuMatrixBase<BaseFloat> &out_value,
                          void *memo);
  virtual Component* Copy() const { return new SoftmaxComponent(*this); }
 private:
  SoftmaxComponent &operator = (const SoftmaxComponent &other); // Disallow.
};


/*
   Implements the log of a softmax nonlinearity, so it's the same
   as shifting each input vector by a constant offset so that, when
   exponentiated, it would sum to one.

   We usually use this in place of softmax because the log-scale
   output will not saturate.

   Configuration values accepted:
      dim            e.g. dim=8061.   Usually this is the last component
                     in a network, so 'dim' is the number of classes.
 */
class LogSoftmaxComponent: public NonlinearComponent {
 public:
  explicit LogSoftmaxComponent(const LogSoftmaxComponent &other):
      NonlinearComponent(other) { }
  LogSoftmaxComponent() { }
  virtual std::string Type() const { return "LogSoftmaxComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsOutput|kStoresStats;
  }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual Component* Copy() const { return new LogSoftmaxComponent(*this); }
 private:
  LogSoftmaxComponent &operator = (const LogSoftmaxComponent &other); // Disallow.
};

/*
  Keywords: natural gradient descent, NG-SGD, naturalgradient.  For
  the top-level of the natural gradient code look here, and also in
  nnet-precondition-online.h.
  NaturalGradientAffineComponent is
  a version of AffineComponent that has a non-(multiple of unit) learning-rate
  matrix.  See nnet-precondition-online.h for a description of the technique.
  It is described, under the name Online NG-SGD, in the paper "Parallel
  training of DNNs with Natural Gradient and Parameter Averaging" (ICLR
  workshop, 2015) by Daniel Povey, Xiaohui Zhang and Sanjeev Khudanpur.

  Configuration values accepted by this component:

  Values inherited from UpdatableComponent (see its declaration in
  nnet-component-itf.h for details):
     learning-rate
     learning-rate-factor
     max-change

  Values used in initializing the component's parameters:
     input-dim             e.g. input-dim=1024.  The input dimension.
     output-dim            e.g. output-dim=1024.  The output dimension.
     param-stddev          e.g. param-stddev=0.025.  The standard deviation
                           used to randomly initialize the linear parameters
                           (as Gaussian random values * param-stddev).
                           Defaults to 1/sqrt(input-dim), which is Glorot
                           initialization.
     bias-stddev           e.g. bias-stddev=0.0.  The standard deviation
                           used to randomly initialize the bias parameters.
                           Defaults to 1.0 but we usually set it to 0.0
                           in the config.
     bias-mean             e.g. bias-mean=1.0.  Allows you to ininialize the
                           bias parameters with an offset.  Default is 0.0
                           which is normally suitable

     matrix                e.g. matrix=foo/bar/init.mat  May be used as an
                           alternative to (input-dim, output-dim, param-stddev,
                           bias-stddev, bias-mean) to initialize the parameters.
                           Dimension is output-dim by (input-dim + 1), last
                           column is interpreted as the bias.

   Other options:
    orthonormal-constraint=0.0   If you set this to 1.0, then
                           the linear_params_ matrix will be (approximately)
                           constrained during training to have orthonormal rows
                           (or columns, whichever is fewer).. it turns out the
                           real name for this is a "semi-orthogonal" matrix.
                           You can choose a positive nonzero value different
                           than 1.0 to have a scaled semi-orthgonal matrix,
                           i.e. with singular values at the selected value
                           (e.g. 0.5, or 2.0).  This is not enforced inside the
                           component itself; you have to call
                           ConstrainOrthonormal() from the training code to do
                           this.  All this component does is return the
                           OrthonormalConstraint() value.  If you set this to a
                           negative value, it's like saying "for any value",
                           i.e. it will constrain the parameter matrix to be
                           closer to "any alpha" times a semi-orthogonal matrix,
                           without changing its overall norm.


   Options to the natural gradient (you won't normally have to set these,
   the defaults are suitable):

      num-samples-history   Number of frames used as the time-constant to
                            determine how 'up-to-date' the Fisher-matrix
                            estimates are.  Smaller -> more up-to-date, but more
                            noisy.  default=2000.
      alpha                 Constant that determines how much we smooth the
                            Fisher-matrix estimates with the unit matrix.
                            Larger means more smoothing. default=4.0
      rank-in               Rank used in low-rank-plus-unit estimate of Fisher
                            matrix in the input space.  default=20.
      rank-out              Rank used in low-rank-plus-unit estimate of Fisher
                            matrix in the output-derivative space.  default=80.
      update-period         Determines the period (in minibatches) with which
                            we update the Fisher-matrix estimates;
                            making this > 1 saves a little time in training.
                            default=4.
*/
class NaturalGradientAffineComponent: public AffineComponent {
 public:
  virtual std::string Type() const { return "NaturalGradientAffineComponent"; }
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  // this constructor does not really initialize, use InitFromConfig() or Read().
  NaturalGradientAffineComponent() { }
  void InitFromConfig(ConfigLine *cfl);
  virtual std::string Info() const;
  virtual Component* Copy() const;
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void FreezeNaturalGradient(bool freeze);

  virtual void ConsolidateMemory();

  // copy constructor
  explicit NaturalGradientAffineComponent(
      const NaturalGradientAffineComponent &other);
  NaturalGradientAffineComponent(
      const CuMatrixBase<BaseFloat> &linear_params,
      const CuVectorBase<BaseFloat> &bias_params);
 private:
  // disallow assignment operator.
  NaturalGradientAffineComponent &operator= (
      const NaturalGradientAffineComponent&);

  OnlineNaturalGradient preconditioner_in_;

  OnlineNaturalGradient preconditioner_out_;

  virtual void Update(
      const std::string &debug_info,
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv);
};

/*
  LinearComponent represents a linear (matrix) transformation of its input, with
  a matrix as its trainable parameters.  It's the same as
  NaturalGradientAffineComponent, but without the bias term.

  Configuration values accepted by this component:

  Values inherited from UpdatableComponent (see its declaration in
  nnet-component-itf for details):
     learning-rate
     learning-rate-factor
     max-change

  Values used in initializing the component's parameters:
     input-dim             e.g. input-dim=1024.  The input dimension.
     output-dim            e.g. output-dim=1024.  The output dimension.
     param-stddev          e.g. param-stddev=0.025.  The standard deviation
                           used to randomly initialize the linear parameters
                           (as Gaussian random values * param-stddev).
                           Defaults to 1/sqrt(input-dim), which is Glorot
                           initialization.
     matrix                e.g. matrix=foo/bar/init.mat  May be used as an
                           alternative to (input-dim, output-dim, param-stddev,
                           bias-stddev, bias-mean) to initialize the parameters.
                           Dimension is output-dim by (input-dim + 1), last
                           column is interpreted as the bias.
    orthonormal-constraint=0.0   If you set this to 1.0, then
                           the linear_params_ matrix will be (approximately)
                           constrained during training to have orthonormal rows
                           (or columns, whichever is fewer).. it turns out the
                           real name for this is a "semi-orthogonal" matrix.
                           You can choose a positive nonzero value different
                           than 1.0 to have a scaled semi-orthgonal matrix,
                           i.e. with singular values at the selected value
                           (e.g. 0.5, or 2.0).  This is not enforced inside the
                           component itself; you have to call
                           ConstrainOrthonormal() from the training code to do
                           this.  All this component does is return the
                           OrthonormalConstraint() value.  If you set this to a
                           negative value, it's like saying "for any value",
                           i.e. it will constrain the parameter matrix to be
                           closer to "any alpha" times a semi-orthogonal matrix,
                           without changing its overall norm.

   Options to the natural gradient (you won't normally have to set these,
   the defaults are suitable):

      use-natural-gradient=true   Set this to false to disable the natural-gradient
                            update entirely (it will do regular SGD).
      num-samples-history   Number of frames used as the time-constant to
                            determine how 'up-to-date' the Fisher-matrix
                            estimates are.  Smaller -> more up-to-date, but more
                            noisy.  default=2000.
      alpha                 Constant that determines how much we smooth the
                            Fisher-matrix estimates with the unit matrix.
                            Larger means more smoothing. default=4.0
      rank-in               Rank used in low-rank-plus-unit estimate of Fisher
                            matrix in the input space.  default=20.
      rank-out              Rank used in low-rank-plus-unit estimate of Fisher
                            matrix in the output-derivative space.  default=80.
      update-period         Determines after with what frequency (in
                            minibatches) we update the Fisher-matrix estimates;
                            making this > 1 saves a little time in training.
                            default=4.
*/
class LinearComponent: public UpdatableComponent {
 public:
  virtual int32 InputDim() const { return params_.NumCols(); }
  virtual int32 OutputDim() const { return params_.NumRows(); }

  virtual std::string Type() const { return "LinearComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kUpdatableComponent|kBackpropNeedsInput|
        kPropagateAdds|kBackpropAdds;
  }

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  // this constructor does not really initialize, use InitFromConfig() or Read().
  LinearComponent() { }
  void InitFromConfig(ConfigLine *cfl);
  virtual std::string Info() const;
  virtual Component* Copy() const;
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const;
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);
  virtual void FreezeNaturalGradient(bool freeze);
  virtual void ConsolidateMemory();

  // copy constructor
  explicit LinearComponent(const LinearComponent &other);

  explicit LinearComponent(const CuMatrix<BaseFloat> &params);

  BaseFloat OrthonormalConstraint() const { return orthonormal_constraint_; }
  CuMatrixBase<BaseFloat> &Params() { return params_; }
  const CuMatrixBase<BaseFloat> &Params() const { return params_; }
 private:

  // disallow assignment operator.
  LinearComponent &operator= (
      const LinearComponent&);

  CuMatrix<BaseFloat> params_;

  BaseFloat orthonormal_constraint_;
  // If true (and if no this->is_gradient_), use natural gradient updates.
  bool use_natural_gradient_;
  OnlineNaturalGradient preconditioner_in_;
  OnlineNaturalGradient preconditioner_out_;
};


/// FixedAffineComponent is an affine transform that is supplied
/// at network initialization time and is not trainable.
class FixedAffineComponent: public Component {
 public:
  FixedAffineComponent() { }
  virtual std::string Type() const { return "FixedAffineComponent"; }
  virtual std::string Info() const;

  // Copy constructor from AffineComponent-- can be used when we're done
  // training a particular part of the model and want to efficiently disable
  // further training.
  FixedAffineComponent(const AffineComponent &c);

  /// matrix should be of size input-dim+1 to output-dim, last col is offset
  void Init(const CuMatrixBase<BaseFloat> &matrix);

  // The ConfigLine cfl contains just the option matrix=<string>,
  // where the string is the filename of a Kaldi-format matrix to read.
  virtual void InitFromConfig(ConfigLine *cfl);

  virtual int32 Properties() const { return kSimpleComponent|kBackpropAdds; }
  virtual int32 InputDim() const { return linear_params_.NumCols(); }
  virtual int32 OutputDim() const { return linear_params_.NumRows(); }

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;


  virtual Component* Copy() const;
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

  const CuMatrix<BaseFloat> &LinearParams() const { return linear_params_; }
  const CuVector<BaseFloat> &BiasParams() const { return bias_params_; }
 protected:
  friend class AffineComponent;
  CuMatrix<BaseFloat> linear_params_;
  CuVector<BaseFloat> bias_params_;

  KALDI_DISALLOW_COPY_AND_ASSIGN(FixedAffineComponent);
};

/// SumGroupComponent is used to sum up groups of posteriors.
/// It's used to introduce a kind of Gaussian-mixture-model-like
/// idea into neural nets.  This is basically a degenerate case of
/// MixtureProbComponent; we had to implement it separately to
/// be efficient for CUDA (we can use this one regardless whether
/// we have CUDA or not; it's the normal case we want anyway).
///
/// There are two forms of initialization in a config file: one
/// where the number of elements are specified for each group
/// individually as a vector, and one where only the total input
/// dimension and the output dimension (number of groups) is specified.
/// The second is used when all groups have the same size.
class SumGroupComponent: public Component {
public:
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  void Init(const std::vector<int32> &sizes); // the vector is of the input dim
                                              // (>= 1) for each output dim.
  void Init(int32 input_dim, int32 output_dim);
  void GetSizes(std::vector<int32> *sizes) const; // Get a vector saying, for
                                                  // each output-dim, how many
                                                  // inputs were summed over.
  virtual void InitFromConfig(ConfigLine *cfl);
  SumGroupComponent() { }
  virtual std::string Type() const { return "SumGroupComponent"; }
  virtual int32 Properties() const { return kSimpleComponent; }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const;
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

private:
  KALDI_DISALLOW_COPY_AND_ASSIGN(SumGroupComponent);
  // Note: Int32Pair is just struct{ int32 first; int32 second }; it's defined
  // in cu-matrixdim.h as extern "C" which is needed for the CUDA interface.
  CuArray<Int32Pair> indexes_; // for each output index, the (start, end) input
                               // index.
  CuArray<int32> reverse_indexes_; // for each input index, the output index.
  int32 input_dim_;
  int32 output_dim_;
};


/// FixedScaleComponent applies a fixed per-element scale; it's similar
/// to the Rescale component in the nnet1 setup (and only needed for nnet1
/// model conversion).
class FixedScaleComponent: public Component {
 public:
  FixedScaleComponent() { }
  virtual std::string Type() const { return "FixedScaleComponent"; }
  virtual std::string Info() const;
  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateInPlace|kBackpropInPlace;
  }

  void Init(const CuVectorBase<BaseFloat> &scales);

  // The ConfigLine cfl contains only the option scales=<string>,
  // where the string is the filename of a Kaldi-format matrix to read.
  virtual void InitFromConfig(ConfigLine *cfl);

  virtual int32 InputDim() const { return scales_.Dim(); }
  virtual int32 OutputDim() const { return scales_.Dim(); }

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, // in_value
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *, // to_update
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const;
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

  const CuVector<BaseFloat> &Scales() const { return scales_; }
 protected:
  CuVector<BaseFloat> scales_;
  KALDI_DISALLOW_COPY_AND_ASSIGN(FixedScaleComponent);
};


/// FixedBiasComponent applies a fixed per-element bias; it's similar
/// to the AddShift component in the nnet1 setup (and only needed for nnet1
/// model conversion.
class FixedBiasComponent: public Component {
 public:
  FixedBiasComponent() { }
  virtual std::string Type() const { return "FixedBiasComponent"; }
  virtual std::string Info() const;

  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateInPlace|kBackpropInPlace;
  }

  void Init(const CuVectorBase<BaseFloat> &scales);

  // The ConfigLine cfl contains only the option bias=<string>,
  // where the string is the filename of a Kaldi-format matrix to read.
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return bias_.Dim(); }
  virtual int32 OutputDim() const { return bias_.Dim(); }
  using Component::Propagate; // to avoid name hiding
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, // in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *, // to_update
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const;
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

 protected:
  CuVector<BaseFloat> bias_;
  KALDI_DISALLOW_COPY_AND_ASSIGN(FixedBiasComponent);
};

/**
   NoOpComponent just duplicates its input.  We don't anticipate this being used
    very often, but it may sometimes make your life easier.  Config parameters:

      dim               E.g. dim=1024.  Required.
      backprop-scale    Defaults to 1.0.  May be set to a different value to scale
                        the derivatives being backpropagated.
*/
class NoOpComponent: public Component {
 public:
  explicit NoOpComponent(const NoOpComponent &other):
      dim_(other.dim_), backprop_scale_(other.backprop_scale_) { }
  NoOpComponent() { }
  virtual std::string Type() const { return "NoOpComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateInPlace|kBackpropInPlace;
  }
  virtual int32 InputDim() const { return dim_; }
  virtual int32 OutputDim() const { return dim_; }
  virtual Component *Copy() { return new NoOpComponent(*this); }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new NoOpComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 dim_;
  BaseFloat backprop_scale_;

  NoOpComponent &operator = (const NoOpComponent &other); // Disallow.
};

/**  SumBlockComponent sums over blocks of its input: for instance, if
     you create one with the config "input-dim=400 output-dim=100",
     its output will be the sum over the 4 100-dimensional blocks of
     the input.

     The "scale" config parameter may be used if you want to do averaging
     instead of summing, e.g. "input-dim=400 output-dim=100 scale=0.25"
     will accomplish averaging.

     Accepted values on its config-file line are:
        input-dim  The input dimension.  Required.
        output-dim  The block dimension.  Required.  Must divide input-dim.
        scale      A scaling factor on the output.  Defaults to 1.0.
 */
class SumBlockComponent: public Component {
 public:
  explicit SumBlockComponent(const SumBlockComponent &other);
  SumBlockComponent() { }
  virtual std::string Type() const { return "SumBlockComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateAdds|kBackpropAdds;
  }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new SumBlockComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  SumBlockComponent &operator = (const SumBlockComponent &other); // Disallow.
};


/*
 ClipGradientComponent just duplicates its input, but clips gradients
 during backpropagation if they cross a predetermined threshold.
 This component will be used to prevent gradient explosion problem in
 recurrent neural networks.

   Configuration values accepted:
      dim                   Dimension of this component, e.g. 1024
      clipping-threshold    Threshold to be used for clipping. It could correspond
                            to max-row-norm (if norm_based_clipping_ == true) or
                            max-absolute-value (otherwise).
      norm-based-clipping   If true, the max-row-norm will be clipped. Else element-wise
                            absolute value clipping is done.
      self-repair-clipped-proportion-threshold  The threshold of clipped-proportion
                            for self-repair mechanism to be activated. The self-repair mechanism
                            adds a term (proportional to [-(input vector - self_repair_target_)])
                            to in-deriv, attempting to shrink the maginitude of the input towards
                            self_repair_target_ (e.g. 0.0 or 0.5). The default value is 1.0.
      self-repair-target    The target value towards which self-repair is trying to set
                            for in-deriv. The default value is 0.0.
      self-repair-scale     Scale for the self-repair mechanism; see comments above.
                            The default value is 0.0, but we usually set this to 1.0e-05 (or
                            occasionally 1.0e-04) in the scripts.
*/

class ClipGradientComponent: public Component {
 public:
  ClipGradientComponent(int32 dim, BaseFloat clipping_threshold,
                        bool norm_based_clipping,
                        BaseFloat self_repair_clipped_proportion_threshold,
                        BaseFloat self_repair_target,
                        BaseFloat self_repair_scale,
                        int32 num_clipped,
                        int32 count,
                        int32 num_self_repaired,
                        int32 num_backpropped) {
    Init(dim, clipping_threshold, norm_based_clipping,
         self_repair_clipped_proportion_threshold,
         self_repair_target,
         self_repair_scale,
         num_clipped, count,
         num_self_repaired, num_backpropped);}

  ClipGradientComponent(): dim_(0), clipping_threshold_(-1),
    norm_based_clipping_(false),
    self_repair_clipped_proportion_threshold_(1.0),
    self_repair_target_(0.0),
    self_repair_scale_(0.0),
    num_clipped_(0), count_(0),
    num_self_repaired_(0), num_backpropped_(0) { }

  virtual int32 InputDim() const { return dim_; }
  virtual int32 OutputDim() const { return dim_; }
  virtual void InitFromConfig(ConfigLine *cfl);
  void Init(int32 dim, BaseFloat clipping_threshold, bool norm_based_clipping,
            BaseFloat self_repair_clipped_proportion_threshold,
            BaseFloat self_repair_target,
            BaseFloat self_repair_scale,
            int32 num_clipped, int32 count,
            int32 num_self_repaired, int32 num_backpropped);

  virtual std::string Type() const { return "ClipGradientComponent"; }

  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateInPlace|kBackpropInPlace|
           kBackpropNeedsInput;
  }

  virtual void ZeroStats();

  virtual Component* Copy() const {
    return new ClipGradientComponent(dim_,
                                     clipping_threshold_,
                                     norm_based_clipping_,
                                     self_repair_clipped_proportion_threshold_,
                                     self_repair_target_,
                                     self_repair_scale_,
                                     num_clipped_,
                                     count_,
                                     num_self_repaired_,
                                     num_backpropped_);}

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.
  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual ~ClipGradientComponent() {
    if (num_self_repaired_ > 0)
      KALDI_LOG << "ClipGradientComponent(node_name=" << debug_info_
                << ")'s self-repair was activated " << num_self_repaired_
                << " time(s) out of " << num_backpropped_
                << " times of calling Backprop() in this training job.";
  }
 private:
  int32 dim_;  // input/output dimension
  BaseFloat clipping_threshold_;  // threshold to be used for clipping
                                  // could correspond to max-row-norm (if
                                  // norm_based_clipping_ == true) or
                                  // max-absolute-value (otherwise)
  bool norm_based_clipping_;  // if true the max-row-norm will be clipped
                              // else element-wise absolute value clipping is
                              // done

  // some configuration values relating to self-repairing.
  BaseFloat self_repair_clipped_proportion_threshold_; // the threshold of
                                                       // clipped-proportion
                                                       // for self-repair to be
                                                       // activated
  BaseFloat self_repair_target_; // the target value towards which self-repair
                                 // is trying to set for in-deriv
  BaseFloat self_repair_scale_;  // constant scaling the self-repair vector
  std::string debug_info_;   // component-node name, used in the destructor to
                             // print out stats of self-repair

  // this function is called from Backprop code, and only does something if the
  // self-repair-scale config value is set and the current clipped proportion
  // exceeds the threshold. What it does is to add a term to in-deriv that
  // forces the input to the ClipGradientComponent to be close to some small
  // value (e.g., 0.0 or 0.5, depending on what the input is, e.g.,
  // Sigmoid or Tanh or Affine). The hope is that if the input is forced to be
  // small, the parameters on the path will also tend to be small, which may
  // help tamp down the divergence caused by gradient explosion.
  void RepairGradients(const std::string &debug_info,
                       const CuMatrixBase<BaseFloat> &in_value,
                       CuMatrixBase<BaseFloat> *in_deriv,
                       ClipGradientComponent *to_update) const;

  ClipGradientComponent &operator =
      (const ClipGradientComponent &other); // Disallow.

 protected:
  // variables to store stats
  // An element corresponds to rows of derivative matrix, when
  // norm_based_clipping_ is true,
  // else it corresponds to each element of the derivative matrix
  // Note: no stats are stored when norm_based_clipping_ is false
  int32 num_clipped_;  // number of elements which were clipped
  int32 count_;  // number of elements which were processed
  int32 num_self_repaired_; // number of times self-repair is activated
  int32 num_backpropped_; //number of times backprop is called

};

/** PermuteComponent changes the order of the columns (i.e. the feature or
    activation dimensions).  Output dimension i is mapped to input dimension
    column_map_[i], so it's like doing:
      for each row:
        for each feature/activation dimension i:
          output(row, i) = input(row, column_map_[i]).

    The only config value it accepts is 'column-map', e.g.:
            column-map=0,10,1,11,...,9,19
    ... which should be a permutation of a contiguous block of integers
    starting with 0 (i.e. something like '3,2,1,0' but not '0,4' or '0,0,2').
    See the equation above for how it is used.
*/
class PermuteComponent: public Component {
 public:
  PermuteComponent()  {}
  PermuteComponent(const std::vector<int32> &column_map) { Init(column_map); }

  virtual int32 InputDim() const { return column_map_.Dim(); }
  virtual int32 OutputDim() const { return column_map_.Dim(); }
  virtual void InitFromConfig(ConfigLine *cfl);
  void Init(const std::vector<int32> &column_map);

  virtual std::string Type() const { return "PermuteComponent"; }

  virtual int32 Properties() const {
    return kSimpleComponent;
  }

  virtual void ZeroStats() {}

  virtual Component* Copy() const;

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual void Scale(BaseFloat scale) {}
  virtual void Add(BaseFloat alpha, const Component &other) {}
  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.
  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
 private:
  // computes the reverse column map.  Must not be called if column_map_.Dim()
  // == 0
  void ComputeReverseColumnMap();
  CuArray<int32> column_map_;
  // the following is a derived variable, not written to disk.
  // It is used in backprop.
  CuArray<int32> reverse_column_map_;
  PermuteComponent &operator =
      (const PermuteComponent &other); // Disallow.
};




/**
   PerElementScaleComponent scales each dimension of its input with a separate
   trainable scale; it's like a linear component with a diagonal matrix.  This
   version (and its child class NaturalGradientPerElementScaleComponent)
   requires the input for backprop.  See also ScaleAndOffsetComponent.

   Accepted values on its config line, with defaults if applicable:

     vector           If specified, the offsets will be read from this file ('vector'
                      is interpreted as an rxfilename).

     dim              The dimension that this component inputs and outputs.
                      Only required if 'vector' is not specified.

     param-mean=1.0   Mean of randomly initialized offset parameters; should only
                      be supplied if 'vector' is not supplied.
     param-stddev=0.0 Standard deviation of randomly initialized offset parameters;
                      should only be supplied if 'vector' is not supplied.
*/
class PerElementScaleComponent: public UpdatableComponent {
 public:
  virtual int32 InputDim() const { return scales_.Dim(); }
  virtual int32 OutputDim() const { return scales_.Dim(); }

  virtual std::string Info() const;
  virtual void InitFromConfig(ConfigLine *cfl);

  PerElementScaleComponent() { } // use Init to really initialize.
  virtual std::string Type() const { return "PerElementScaleComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kUpdatableComponent|kBackpropNeedsInput|
        kPropagateInPlace|kBackpropInPlace;
  }

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

  virtual Component* Copy() const;


  // Some functions from base-class UpdatableComponent.
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const;
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);

  // Some functions that are specific to this class.
  explicit PerElementScaleComponent(const PerElementScaleComponent &other);

  void Init(int32 dim, BaseFloat param_mean, BaseFloat param_stddev);
  void Init(std::string vector_filename);

 protected:
  // This function Update() is for extensibility; child classes may override
  // this, e.g. for natural gradient update.
  virtual void Update(
      const std::string &debug_info,
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv) {
    UpdateSimple(in_value, out_deriv);
  }
  // UpdateSimple is used when *this is a gradient.  Child classes may override
  // this if needed, but typically won't need to.
  virtual void UpdateSimple(
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv);

  const PerElementScaleComponent &operator
      = (const PerElementScaleComponent &other); // Disallow.
  CuVector<BaseFloat> scales_;
};

/*
  PerElementOffsetComponent offsets each dimension of its input with a separate
  trainable bias; it's like an affine component with fixed weight matrix which
  is always equal to I.

  Accepted values on its config line, with defaults if applicable:

     vector           If specified, the offsets will be read from this file ('vector'
                      is interpreted as an rxfilename).

     dim              The dimension that this component inputs and outputs.

     block-dim        [Should not be specified if you specify 'vector'].
                      If specified, must be nonzero and divide 'dim'.  In this
                      case, blocks of the input of this dimension will get
                      the same offset.  Useful in CNNs.

     param-mean=0.0   Mean of randomly initialized offset parameters; should only
                      be supplied if 'vector' is not supplied.
     param-stddev=0.0 Standard deviation of randomly initialized offset parameters;
                      should only be supplied if 'vector' is not supplied.

     use-natural-gradient=true  If true, we will use natural gradient in the
                      update.  Note: this is different from PerElementScaleComponent,
                      which does not support natural gradient directly-- in that
                      case you have to use NaturalGradientPerElementScaleComponent
                      if you want to use natural gradient update.

  Values inherited from UpdatableComponent (see its declaration in
  nnet-component-itf for details):
     learning-rate
     learning-rate-factor
     max-change
*/
class PerElementOffsetComponent: public UpdatableComponent {
 public:
  virtual int32 InputDim() const { return dim_; }
  virtual int32 OutputDim() const { return dim_; }

  virtual std::string Info() const;
  virtual void InitFromConfig(ConfigLine *cfl);

  PerElementOffsetComponent() { } // use Init to really initialize.
  virtual std::string Type() const { return "PerElementOffsetComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kUpdatableComponent|
           kBackpropInPlace|kPropagateInPlace|
        (dim_ != offsets_.Dim() ? kOutputContiguous : 0);
  }

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, // in_value
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

  virtual Component* Copy() const;

  // Some functions from base-class UpdatableComponent.
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const;
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);

  // Copy constructor
  explicit PerElementOffsetComponent(const PerElementOffsetComponent &other);
 protected:
  const PerElementOffsetComponent &operator
      = (const PerElementOffsetComponent &other); // Disallow.
  CuVector<BaseFloat> offsets_;
  // dim_ will normally be the same as offsets_ dim, but in general will be an
  // integer multiple of it (in case the same offset vector is applied to
  // successive blocks of the input).
  int32 dim_;
  bool use_natural_gradient_;
  OnlineNaturalGradient preconditioner_;
};


// ConstantFunctionComponent returns constant function of its input,
// i.e. its output does not depend on its input.  It is the same as
// an affine component with the linear term fixed at zero.
// It is optionally trainable, and optionally you can use natural
// gradient.  The input is required only because it's more convenient
// to make SimpleComponents [but see ConstantComponent, which requires
// no inputs].
class ConstantFunctionComponent: public UpdatableComponent {
 public:
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_.Dim(); }

  virtual std::string Info() const;
  // possible parameter values with their defaults:
  // input-dim=-1 is-updatable=true use-natural-gradient=true output-dim=-1
  // output-mean=0 output-stddev=0
  virtual void InitFromConfig(ConfigLine *cfl);

  ConstantFunctionComponent();

  ConstantFunctionComponent(const ConstantFunctionComponent &other);

  virtual std::string Type() const { return "ConstantFunctionComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|
        (is_updatable_ ? kUpdatableComponent : 0) |
        (InputDim() == OutputDim() ? kPropagateInPlace: 0) |
        kBackpropAdds;
  }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, // in_value
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

  virtual Component* Copy() const;

  // Some functions from base-class UpdatableComponent.
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const;
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);
  virtual void ConsolidateMemory();
 private:
  int32 input_dim_;
  // the output value-- a vector.
  CuVector<BaseFloat> output_;

  bool is_updatable_;
  // if true, and if updatable, do natural-gradient update.
  bool use_natural_gradient_;
  OnlineNaturalGradient preconditioner_;

  const ConstantFunctionComponent &operator
  = (const ConstantFunctionComponent &other); // Disallow.
};



/**
   NaturalGradientPerElementScaleComponent is like PerElementScaleComponent but
   it uses a natural gradient update for the per-element scales.

   Accepted values on its config line, with defaults if applicable:

     vector           If specified, the offsets will be read from this file ('vector'
                      is interpreted as an rxfilename).

     dim              The dimension that this component inputs and outputs.
                      Only required if 'vector' is not specified.

     param-mean=1.0   Mean of randomly initialized offset parameters; should only
                      be supplied if 'vector' is not supplied.
     param-stddev=0.0 Standard deviation of randomly initialized offset parameters;
                      should only be supplied if 'vector' is not supplied.

  And the natural-gradient-related configuration values:
      rank=8
      update-period=10
      num-samples-history=2000.0
      alpha=4.0
*/
class NaturalGradientPerElementScaleComponent: public PerElementScaleComponent {
 public:

  virtual std::string Info() const;

  virtual void InitFromConfig(ConfigLine *cfl);

  NaturalGradientPerElementScaleComponent() { } // use Init to really initialize.
  virtual std::string Type() const {
    return "NaturalGradientPerElementScaleComponent";
  }

  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual void FreezeNaturalGradient(bool freeze);

  virtual Component* Copy() const;

  // Some functions that are specific to this class:
  explicit NaturalGradientPerElementScaleComponent(
      const NaturalGradientPerElementScaleComponent &other);

  void Init(int32 dim, BaseFloat param_mean,
            BaseFloat param_stddev, int32 rank, int32 update_period,
            BaseFloat num_samples_history, BaseFloat alpha);
  void Init(std::string vector_filename,
            int32 rank, int32 update_period, BaseFloat num_samples_history,
            BaseFloat alpha);

  void ConsolidateMemory();

 private:
  // unlike the NaturalGradientAffineComponent, there is only one dimension to
  // consider as the parameters are a vector not a matrix, so we only need one
  // preconditioner.
  // The preconditioner stores its own configuration values; we write and read
  // these, but not the preconditioner object itself.
  OnlineNaturalGradient preconditioner_;

  // Override of the parent-class Update() function, called only
  // if this->is_gradient_ = false; this implements the natural
  // gradient update.
  virtual void Update(
      const std::string &debug_info,
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv);

  const NaturalGradientPerElementScaleComponent &operator
      = (const NaturalGradientPerElementScaleComponent &other); // Disallow.
};


/*
  ScaleAndOffsetComponent implements a per-element scale and offset.
  It may be useful just after BatchNormComponent, as the trainable offset
  and scale of batch-norm.
  Note: by default this includes natural gradient for the update.

  Currently accepted values on its config line are as follows.
  Major configuration values:

     dim              The feature-dimension that the component takes as
                      input, and outputs.
     block-dim        If set, this must be set to a value that divides
                      'dim'.  In this case, the same offset and scale
                      will be applied to each block, and the number
                      of parameters will be 2*block-dim instead of 2*dim.

  There is currently no way to configure what values will be used for
  the initialization and it is hardcoded to zero offset, unit scale.
  If in future more configurability is needed, we'll address it then.

  Values inherited from UpdatableComponent (see its declaration in
  nnet-component-itf for details):
     learning-rate
     learning-rate-factor
     max-change


   Options to the natural gradient (you won't normally have to set these,
   the defaults are suitable):

      use-natural-gradient  Defaults to true; false turns off the application
                            of natural gradient update to this layer.
      rank                  Rank used in low-rank-plus-unit estimate of Fisher
                            matrix in the input space.  default=20.
*/
class ScaleAndOffsetComponent: public UpdatableComponent {
 public:
  virtual int32 InputDim() const { return dim_; }
  virtual int32 OutputDim() const { return dim_; }

  virtual std::string Info() const;
  virtual void InitFromConfig(ConfigLine *cfl);

  ScaleAndOffsetComponent() { } // use Init to really initialize.
  virtual std::string Type() const { return "ScaleAndOffsetComponent"; }
  virtual int32 Properties() const {
    // Note: the backprop would most naturally consume the input, but we
    // have arranged things so that the backprop consumes the output value
    // instead; this allows less memory use, since in typical configurations,
    // this will be followed by an affine component which needs its input
    // for the backprop (so requiring it to be present adds no extra
    // burden).
    return kSimpleComponent|kUpdatableComponent|
           kBackpropInPlace|kPropagateInPlace|
           kBackpropNeedsOutput|
        (dim_ != scales_.Dim() ?
         (kInputContiguous|kOutputContiguous) : 0);
  }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, // in_value
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

  virtual Component* Copy() const { return new ScaleAndOffsetComponent(*this); }

  // Some functions from base-class UpdatableComponent.
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const { return 2 * scales_.Dim(); }
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);
  virtual void ConsolidateMemory();


  // copy constructor
  explicit ScaleAndOffsetComponent(const ScaleAndOffsetComponent &other);
 private:
  // Internal version of propagate, requires in.NumCols() equal to scales_.Dim()
  // (if batch-dim was set, this may require the caller to reshape the input and
  // output.
  void PropagateInternal(const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  // Internal version of backprop, where the num-cols of the
  // argument matrices are equal to scales_.Dim().
  void BackpropInternal(const std::string &debug_info,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        ScaleAndOffsetComponent *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  // We do this instead of defining a constant, which is a hassle in C++.
  inline BaseFloat Epsilon() const { return 1.0e-04; }

  // called from BackpropInternal if 'to_update' is non-NULL.
  void Update(
      const std::string &debug_info,
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv);


  const ScaleAndOffsetComponent &operator
      = (const ScaleAndOffsetComponent &other); // Disallow.

  // Note: dim_ is the dimension that the component takes as input
  // and output.  It is an integer multiple of scales_.Dim(),
  // and will be the same as scales_.Dim() unless 'block-dim'
  // was specified on the config line.
  // (note: scales_.Dim() and offset_.Dim() will be the same).
  int32 dim_;

  // note: output is y(i) = scales_(i) * x(i) + offsets_(i).
  CuVector<BaseFloat> scales_;
  CuVector<BaseFloat> offsets_;
  bool use_natural_gradient_;
  OnlineNaturalGradient scale_preconditioner_;
  OnlineNaturalGradient offset_preconditioner_;
};


/**
   CompositeComponent is a component representing a sequence of
   [simple] components.  The config line would be something like the following
   (imagine this is all on one line):

   component name=composite1 type=CompositeComponent max-rows-process=2048 num-components=3 \
      component1='type=BlockAffineComponent input-dim=1000 output-dim=10000 num-blocks=100' \
      component2='type=RectifiedLinearComponent dim=10000' \
      component3='type=BlockAffineComponent input-dim=10000 output-dim=1000 num-blocks=100'

   The reason you might want to use this component, instead of directly using
   the same sequence of components in the config file, is to save GPU memory (at
   the expense of more compute)-- because doing it like this means we have to
   re-do parts of the forward pass in the backprop phase, but we avoid using
   much memory for very long (and you can make the memory usage very small by
   making max-rows-process small).  We inherit from UpdatableComponent just in
   case one or more of the components in the sequence are updatable.

   It is an error to nest a CompositeComponent inside a CompositeComponent.
   The same effect can be accomplished by specifying a smaller max-rows-process
   in a single CompositeComponent.
 */
class CompositeComponent: public UpdatableComponent {
 public:
  virtual int32 InputDim() const;
  virtual int32 OutputDim() const;

  virtual std::string Info() const;

  virtual void InitFromConfig(ConfigLine *cfl);

  virtual Component* Copy() const;

  CompositeComponent() { } // use Init() or InitFromConfig() to really initialize.

  // Initialize from this list of components; takes ownership of the pointers.
  void Init(const std::vector<Component*> &components,
            int32 max_rows_process);

  virtual std::string Type() const { return "CompositeComponent"; }

  // The properties depend on the properties of the constituent components.  As
  // a special case, we never return kStoresStats in the properties: by default
  // we store things like activation stats (e.g. for nonlinear components like
  // ReLU) as part of the backprop.  This means we may wastefully store stats
  // even when not requested, but it does save time as a separate StoreStats()
  // call would involve propagating the internals.
  virtual int32 Properties() const;

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  // note, we don't implement StoreStats() as it would be inefficient.  Instead,
  // by default we call StoreStats() on all members that have the flag set,
  // inside the Backprop.
  virtual void ZeroStats();

  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

  // Don't implement Copy() at this level: implement it in the child class.

  // Some functions from base-class UpdatableComponent.
  virtual void SetUnderlyingLearningRate(BaseFloat lrate);
  virtual void SetActualLearningRate(BaseFloat lrate);
  virtual void SetAsGradient();
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const;
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);
  virtual void FreezeNaturalGradient(bool freeze);

  // note: we dont implement the StoreStats function as it would be quite
  // expensive; instead, by default we call StoreStats() for any components that
  // want to store stats, as part of the backprop pass.  This is not 100% ideal
  // but it will usually do what you want.  We can revisit this later if needed.

  // Functions to iterate over the internal components

  int32 NumComponents() const { return components_.size(); }
  /// Gets the ith component in this component.
  /// The ordering is the same as in the config line. The caller
  /// does not own the received component.
  const Component* GetComponent(int32 i) const;
  /// Sets the ith component. After this call, CompositeComponent owns
  /// the reference to the argument component. Frees the previous
  /// ith component.
  void SetComponent(int32 i, Component *component);

  virtual ~CompositeComponent() { DeletePointers(&components_); }
 private:
  // returns the stride type, kDefaultStride or kStrideEqualNumCols,
  // at the output of the i'th component.
  inline MatrixStrideType GetStrideType(int32 i) const;

  // returns true if at least one of 'components_' returns the kUpdatable flag
  // in its flags.
  bool IsUpdatable() const;

  // the maximum number of
  int32 max_rows_process_;
  std::vector<Component*> components_;

};


// Modified version for adaptation!!!

//////////

/**
   NormalRandComponent just samples from a std normal dist, omitting the input:

      dim               E.g. dim=1024.  Required.
*/
class NormalRandComponent: public RandomComponent {
 public:
  void Init(int32 dim, int32 input_dim, bool rand_per_frame);
  NormalRandComponent(int32 dim, int32 input_dim, bool rand_per_frame) {
    Init(dim, input_dim, rand_per_frame);
  }
  NormalRandComponent(): dim_(0), input_dim_(0), rand_per_frame_(false) { }
  explicit NormalRandComponent(const NormalRandComponent &other):
      dim_(other.dim_), input_dim_(other.input_dim_), rand_per_frame_(other.rand_per_frame_) { }
  virtual std::string Type() const { return "NormalRandComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kRandomComponent;
  }
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return dim_; }
  virtual Component *Copy() { return new NormalRandComponent(*this); }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new NormalRandComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 dim_;
  int32 input_dim_;
  bool rand_per_frame_;
  
  NormalRandComponent &operator = (const NormalRandComponent &other); // Disallow.
};

//////////

/**
   NormalRandFComponent just samples from a std normal dist, omitting the input:

      dim               E.g. dim=1024.  Required.
*/
class NormalRandFComponent: public RandomComponent {
 public:
  void Init(int32 dim, int32 input_dim, bool rand_per_frame);
  NormalRandFComponent(int32 dim, int32 input_dim, bool rand_per_frame) {
    Init(dim, input_dim, rand_per_frame);
  }
  NormalRandFComponent(): dim_(0), input_dim_(0), rand_per_frame_(false) { }
  explicit NormalRandFComponent(const NormalRandFComponent &other):
      dim_(other.dim_), input_dim_(other.input_dim_), rand_per_frame_(other.rand_per_frame_) { }
  virtual std::string Type() const { return "NormalRandFComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kRandomComponent;
  }
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return dim_; }
  virtual Component *Copy() { return new NormalRandFComponent(*this); }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new NormalRandFComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 dim_;
  int32 input_dim_;
  bool rand_per_frame_;
  
  NormalRandFComponent &operator = (const NormalRandFComponent &other); // Disallow.
};


//////////

class DropoutMaskSimpleComponent: public RandomComponent {
 public:
  void Init(int32 dim, int32 input_dim, bool rand_per_frame, BaseFloat rand_num);
  DropoutMaskSimpleComponent(int32 dim, int32 input_dim, bool rand_per_frame, BaseFloat rand_num) {
    Init(dim, input_dim, rand_per_frame, rand_num);
  }
  DropoutMaskSimpleComponent(): dim_(0), input_dim_(0), rand_per_frame_(false), rand_num_(0.5) { }
  explicit DropoutMaskSimpleComponent(const DropoutMaskSimpleComponent &other):
      dim_(other.dim_), input_dim_(other.input_dim_), rand_per_frame_(other.rand_per_frame_), rand_num_(other.rand_num_) { }
  virtual std::string Type() const { return "DropoutMaskSimpleComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kRandomComponent;
  }
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return dim_; }
  virtual Component *Copy() { return new DropoutMaskSimpleComponent(*this); }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new DropoutMaskSimpleComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 dim_;
  int32 input_dim_;
  bool rand_per_frame_;
  BaseFloat rand_num_;
  
  DropoutMaskSimpleComponent &operator = (const DropoutMaskSimpleComponent &other); // Disallow.
};

//////////

class RandSelectNComponent: public RandomComponent {
 public:
  void Init(int32 dim, int32 input_dim, bool rand_per_frame, int32 rand_num);
  RandSelectNComponent(int32 dim, int32 input_dim, bool rand_per_frame, int32 rand_num) {
    Init(dim, input_dim, rand_per_frame, rand_num);
  }
  RandSelectNComponent(): dim_(0), input_dim_(0), rand_per_frame_(false), rand_num_(1) { }
  explicit RandSelectNComponent(const RandSelectNComponent &other):
      dim_(other.dim_), input_dim_(other.input_dim_), rand_per_frame_(other.rand_per_frame_), rand_num_(other.rand_num_) { }
  virtual std::string Type() const { return "RandSelectNComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kRandomComponent;
  }
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return dim_; }
  virtual Component *Copy() { return new RandSelectNComponent(*this); }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new RandSelectNComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 dim_;
  int32 input_dim_;
  bool rand_per_frame_;
  int32 rand_num_;
  
  RandSelectNComponent &operator = (const RandSelectNComponent &other); // Disallow.
};

//////////

/**
   NormalRandMultiSampleComponent just samples from a std normal dist, omitting the input:

      dim               E.g. dim=1024.  Required.
*/
class NormalRandMultiSampleComponent: public RandomComponent {
 public:
  void Init(int32 dim, int32 input_dim, bool rand_per_frame, bool test_time_sample);
  NormalRandMultiSampleComponent(int32 dim, int32 input_dim, bool rand_per_frame, bool test_time_sample) {
    Init(dim, input_dim, rand_per_frame, test_time_sample);
  }
  NormalRandMultiSampleComponent(): dim_(0), input_dim_(0), rand_per_frame_(false), test_time_sample_(false) { }
  explicit NormalRandMultiSampleComponent(const NormalRandMultiSampleComponent &other):
      dim_(other.dim_), input_dim_(other.input_dim_), rand_per_frame_(other.rand_per_frame_), test_time_sample_(other.test_time_sample_) { }
  virtual std::string Type() const { return "NormalRandMultiSampleComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kRandomComponent;
  }
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return dim_; }
  virtual Component *Copy() { return new NormalRandMultiSampleComponent(*this); }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new NormalRandMultiSampleComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 dim_;
  int32 input_dim_;
  bool rand_per_frame_;
  bool test_time_sample_;
  
  NormalRandMultiSampleComponent &operator = (const NormalRandMultiSampleComponent &other); // Disallow.
};

//////////

class TestZeroComponent: public RandomComponent {
 public:
  void Init(int32 dim, int32 input_dim);
  TestZeroComponent(int32 dim, int32 input_dim) {
    Init(dim, input_dim);
  }
  TestZeroComponent(): dim_(0), input_dim_(0) { }
  explicit TestZeroComponent(const TestZeroComponent &other):
      dim_(other.dim_), input_dim_(other.input_dim_) { }
  virtual std::string Type() const { return "TestZeroComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kRandomComponent;
  }
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return dim_; }
  virtual Component *Copy() { return new TestZeroComponent(*this); }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new TestZeroComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 dim_;
  int32 input_dim_;
  
  TestZeroComponent &operator = (const TestZeroComponent &other); // Disallow.
};

//////////

class CopyNComponent: public Component {
 public:
  explicit CopyNComponent(const CopyNComponent &other);
  CopyNComponent() { }
  virtual std::string Type() const { return "CopyNComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateAdds|kBackpropAdds;
  }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new CopyNComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  CopyNComponent &operator = (const CopyNComponent &other); // Disallow.
};


//////////

class CopyNPerDimComponent: public Component {
 public:
  explicit CopyNPerDimComponent(const CopyNPerDimComponent &other);
  CopyNPerDimComponent() { }
  virtual std::string Type() const { return "CopyNPerDimComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateAdds|kBackpropAdds;
  }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new CopyNPerDimComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  int32 copy_num_;
  CopyNPerDimComponent &operator = (const CopyNPerDimComponent &other); // Disallow.
};

//////////

class DimRangeComponent: public Component {
 public:
  explicit DimRangeComponent(const DimRangeComponent &other);
  DimRangeComponent() { }
  virtual std::string Type() const { return "DimRangeComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent;
  }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new DimRangeComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 input_dim_;
  int32 output_dim_;
  int32 dim_offset_;
  DimRangeComponent &operator = (const DimRangeComponent &other); // Disallow.
};


//////////

/**
   MinValueComponent just minimizes a vector of values.  Config parameters:

      dim               E.g. dim=1.  Required.
      scale    Defaults to 1.0. 
*/
class MinValueComponent: public Component {
 public:
  explicit MinValueComponent(const MinValueComponent &other):
      dim_(other.dim_), scale_(other.scale_), report_step_(other.report_step_), count_(other.count_), sum_(other.sum_), test_mode_(other.test_mode_) { }
  MinValueComponent() { }
  
  void SetTestMode(bool test_mode) { test_mode_ = test_mode; }
  
  virtual std::string Type() const { return "MinValueComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateInPlace|kUsesMemo|kStoresStats;
  }
  virtual int32 InputDim() const { return dim_; }
  virtual int32 OutputDim() const { return dim_; }
  virtual Component *Copy() { return new MinValueComponent(*this); }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new MinValueComponent(*this); }
  
  virtual void StoreStats(const CuMatrixBase<BaseFloat> &in_value,
                          const CuMatrixBase<BaseFloat> &out_value,
                          void *memo);
  virtual void ZeroStats();
  
  virtual void DeleteMemo(void *memo) const { delete static_cast<Memo*>(memo); }
  
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 dim_;
  BaseFloat scale_;
  int32 report_step_;
  
  struct Memo {
	int32 count;
    double sum;
  };
  
  int32 count_;
  double sum_;
  
  bool test_mode_;

  MinValueComponent &operator = (const MinValueComponent &other); // Disallow.
};


//////////

class ExpComponent: public NonlinearComponent {
 public:
  explicit ExpComponent(const ExpComponent &other): NonlinearComponent(other) { }
  ExpComponent() { }
  virtual std::string Type() const { return "ExpComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsOutput|kPropagateInPlace|kStoresStats;
  }
  virtual Component* Copy() const { return new ExpComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual void StoreStats(const CuMatrixBase<BaseFloat> &in_value,
                          const CuMatrixBase<BaseFloat> &out_value,
                          void *memo);
 private:

  ExpComponent &operator = (const ExpComponent &other); // Disallow.
};

//////////

class LogComponent: public NonlinearComponent {
 public:
  explicit LogComponent(const LogComponent &other): NonlinearComponent(other) { }
  LogComponent() { }
  virtual std::string Type() const { return "LogComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kPropagateInPlace|kStoresStats;
  }
  virtual Component* Copy() const { return new LogComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, //out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual void StoreStats(const CuMatrixBase<BaseFloat> &in_value,
                          const CuMatrixBase<BaseFloat> &out_value,
                          void *memo);
 private:

  LogComponent &operator = (const LogComponent &other); // Disallow.
};


/////////////////////////////

class SoftReLUComponent: public NonlinearComponent {
 public:
  explicit SoftReLUComponent(const SoftReLUComponent &other):
      NonlinearComponent(other) { }
  SoftReLUComponent() { }
  virtual std::string Type() const { return "SoftReLUComponent"; }
  virtual Component* Copy() const { return new SoftReLUComponent(*this); }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kPropagateInPlace|
        kStoresStats;
  }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, //out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual void StoreStats(const CuMatrixBase<BaseFloat> &in_value,
                          const CuMatrixBase<BaseFloat> &out_value,
                          void *memo);
 private:

  SoftReLUComponent &operator = (const SoftReLUComponent &other); // Disallow.
};

//////////

class FloorReLUComponent: public NonlinearComponent {
 public:
  explicit FloorReLUComponent(const FloorReLUComponent &other): NonlinearComponent(other) { }
  FloorReLUComponent() { }
  virtual std::string Type() const { return "FloorReLUComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsOutput|kPropagateInPlace|kStoresStats;
  }
  virtual Component* Copy() const { return new FloorReLUComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual void StoreStats(const CuMatrixBase<BaseFloat> &in_value,
                          const CuMatrixBase<BaseFloat> &out_value,
                          void *memo);
 private:

  FloorReLUComponent &operator = (const FloorReLUComponent &other); // Disallow.
};

////////////////////////////////

class KLGaussianComponent: public Component {
 public:
 /*
  void SetTermps(const CuMatrixBase<BaseFloat> &inv_prior_std,
                 const CuMatrixBase<BaseFloat> &diff,
  			   const CuMatrixBase<BaseFloat> &diff_power_2,
  			   const CuMatrixBase<BaseFloat> &rate) const;
  void SetTermps(const CuMatrixBase<BaseFloat> &inv_prior_std,
                 const CuMatrixBase<BaseFloat> &diff,
  			   const CuMatrixBase<BaseFloat> &diff_power_2,
  			   const CuMatrixBase<BaseFloat> &rate,
			   const CuVectorBase<BaseFloat> &frame_scale) const;
  */
  
  void Init(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale, bool output_sum, bool has_output);
  explicit KLGaussianComponent(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale, bool output_sum, bool has_output) {
    Init(input_dim, output_dim, scale, input_frame_scale, inv_frame_scale, output_sum, has_output);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kBackpropNeedsOutput;
  }
  KLGaussianComponent(): input_dim_(0), output_dim_(0), scale_(1.0), input_frame_scale_(false), inv_frame_scale_(false), output_sum_(false), has_output_(true) { }
  virtual std::string Type() const { return "KLGaussianComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual std::string Info() const;
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new KLGaussianComponent(input_dim_, output_dim_, scale_, input_frame_scale_, inv_frame_scale_, output_sum_, has_output_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

 protected:
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  bool input_frame_scale_;
  bool inv_frame_scale_;
  bool output_sum_;
  bool has_output_;
  
  // CuMatrix<BaseFloat> *inv_prior_std_(int32 num_rows, int32 num_cols) const;
  // CuMatrix<BaseFloat> *diff_(int32 num_rows, int32 num_cols) const;
  // CuMatrix<BaseFloat> *diff_power_2_(int32 num_rows, int32 num_cols) const;
  // CuMatrix<BaseFloat> *rate_(int32 num_rows, int32 num_cols) const;
  // CuVector<BaseFloat> *frame_scale_(int32 num_rows) const;
};

////////////////////////////////

class KLGaussianLogStdComponent: public Component {
 public:
 /*
  void SetTermps(const CuMatrixBase<BaseFloat> &inv_prior_std,
                 const CuMatrixBase<BaseFloat> &diff,
  			   const CuMatrixBase<BaseFloat> &diff_power_2,
  			   const CuMatrixBase<BaseFloat> &rate) const;
  void SetTermps(const CuMatrixBase<BaseFloat> &inv_prior_std,
                 const CuMatrixBase<BaseFloat> &diff,
  			   const CuMatrixBase<BaseFloat> &diff_power_2,
  			   const CuMatrixBase<BaseFloat> &rate,
			   const CuVectorBase<BaseFloat> &frame_scale) const;
  */
  
  void Init(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale, bool output_sum, bool has_output);
  explicit KLGaussianLogStdComponent(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale, bool output_sum, bool has_output) {
    Init(input_dim, output_dim, scale, input_frame_scale, inv_frame_scale, output_sum, has_output);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kBackpropNeedsOutput;
  }
  KLGaussianLogStdComponent(): input_dim_(0), output_dim_(0), scale_(1.0), input_frame_scale_(false), inv_frame_scale_(false), output_sum_(false), has_output_(true) { }
  virtual std::string Type() const { return "KLGaussianLogStdComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual std::string Info() const;
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new KLGaussianLogStdComponent(input_dim_, output_dim_, scale_, input_frame_scale_, inv_frame_scale_, output_sum_, has_output_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

 protected:
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  bool input_frame_scale_;
  bool inv_frame_scale_;
  bool output_sum_;
  bool has_output_;
  
  // CuMatrix<BaseFloat> *inv_prior_std_(int32 num_rows, int32 num_cols) const;
  // CuMatrix<BaseFloat> *diff_(int32 num_rows, int32 num_cols) const;
  // CuMatrix<BaseFloat> *diff_power_2_(int32 num_rows, int32 num_cols) const;
  // CuMatrix<BaseFloat> *rate_(int32 num_rows, int32 num_cols) const;
  // CuVector<BaseFloat> *frame_scale_(int32 num_rows) const;
};

////////////////////////////////

class BayesVecKLGaussianLogStdComponent: public RandomComponent {
 public:
  void Init(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale, bool output_sum, bool has_output);
  explicit BayesVecKLGaussianLogStdComponent(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale, bool output_sum, bool has_output) {
    Init(input_dim, output_dim, scale, input_frame_scale, inv_frame_scale, output_sum, has_output);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kBackpropNeedsOutput|kUsesMemo|kRandomComponent;
  }
  BayesVecKLGaussianLogStdComponent(): input_dim_(0), output_dim_(0), KL_scale_(1.0), input_frame_scale_(false), inv_frame_scale_(false), rand_per_frame_(false), KL_output_(true) { }
  virtual std::string Type() const { return "BayesVecKLGaussianLogStdComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual std::string Info() const;
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new BayesVecKLGaussianLogStdComponent(input_dim_, output_dim_, KL_scale_, input_frame_scale_, inv_frame_scale_, rand_per_frame_, KL_output_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

  virtual void DeleteMemo(void *memo) const { delete static_cast<Memo*>(memo); }
  
 protected:
  
  struct Memo {
	CuVector<BaseFloat> rand_vec;
	CuMatrix<BaseFloat> rand_mat;
	CuMatrix<BaseFloat> inv_prior_std;
	CuMatrix<BaseFloat> post_std;
	CuMatrix<BaseFloat> diff;
	CuMatrix<BaseFloat> diff_power_2;
	CuMatrix<BaseFloat> rate_power_2;
	CuVector<BaseFloat> frame_scale;
  };
  
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat KL_scale_;
  bool input_frame_scale_;
  bool inv_frame_scale_;
  bool KL_output_;
  bool rand_per_frame_;
};

////////////////////////////////

class BayesVecKLGaussianComponent: public RandomComponent {
 public:
  void Init(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale, bool rand_per_frame, bool KL_output, bool fix_rand_val);
  explicit BayesVecKLGaussianComponent(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale, bool rand_per_frame, bool KL_output, bool fix_rand_val) {
    Init(input_dim, output_dim, scale, input_frame_scale, inv_frame_scale, rand_per_frame, KL_output, fix_rand_val);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kBackpropNeedsOutput|kUsesMemo|kRandomComponent;
  }
  BayesVecKLGaussianComponent(): input_dim_(0), output_dim_(0), KL_scale_(1.0), input_frame_scale_(false), inv_frame_scale_(false), rand_per_frame_(false), KL_output_(true), fix_rand_val_(false) { }
  virtual std::string Type() const { return "BayesVecKLGaussianComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual std::string Info() const;
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new BayesVecKLGaussianComponent(input_dim_, output_dim_, KL_scale_, input_frame_scale_, inv_frame_scale_, rand_per_frame_, KL_output_, fix_rand_val_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

  virtual void DeleteMemo(void *memo) const { delete static_cast<Memo*>(memo); }
  
 protected:
  
  struct Memo {
	CuVector<BaseFloat> rand_vec;
	CuMatrix<BaseFloat> rand_mat;
	CuMatrix<BaseFloat> inv_prior_std;
	CuMatrix<BaseFloat> diff;
	CuMatrix<BaseFloat> diff_power_2;
	CuMatrix<BaseFloat> rate;
	CuMatrix<BaseFloat> rate_power_2;
	CuVector<BaseFloat> frame_scale;
	CuMatrix<BaseFloat> KL_out;
  };
  
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat KL_scale_;
  bool input_frame_scale_;
  bool inv_frame_scale_;
  bool KL_output_;
  bool rand_per_frame_;
  bool fix_rand_val_;
};

////////////////////////////////

class BayesVecKLGaussianFComponent: public RandomComponent {
 public:
  void Init(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale, bool rand_per_frame, bool KL_output, bool fix_rand_val);
  explicit BayesVecKLGaussianFComponent(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale, bool rand_per_frame, bool KL_output, bool fix_rand_val) {
    Init(input_dim, output_dim, scale, input_frame_scale, inv_frame_scale, rand_per_frame, KL_output, fix_rand_val);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kBackpropNeedsOutput|kUsesMemo|kRandomComponent;
  }
  BayesVecKLGaussianFComponent(): input_dim_(0), output_dim_(0), KL_scale_(1.0), input_frame_scale_(false), inv_frame_scale_(false), rand_per_frame_(false), KL_output_(true), fix_rand_val_(false) { }
  virtual std::string Type() const { return "BayesVecKLGaussianFComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual std::string Info() const;
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new BayesVecKLGaussianFComponent(input_dim_, output_dim_, KL_scale_, input_frame_scale_, inv_frame_scale_, rand_per_frame_, KL_output_, fix_rand_val_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

  virtual void DeleteMemo(void *memo) const { delete static_cast<Memo*>(memo); }
  
 protected:
  
  struct Memo {
	CuVector<BaseFloat> rand_vec;
	CuMatrix<BaseFloat> rand_mat;
	CuMatrix<BaseFloat> inv_prior_std;
	CuMatrix<BaseFloat> diff;
	CuMatrix<BaseFloat> diff_power_2;
	CuMatrix<BaseFloat> rate;
	CuMatrix<BaseFloat> rate_power_2;
	CuVector<BaseFloat> frame_scale;
	CuMatrix<BaseFloat> KL_out;
  };
  
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat KL_scale_;
  bool input_frame_scale_;
  bool inv_frame_scale_;
  bool KL_output_;
  bool rand_per_frame_;
  bool fix_rand_val_;
};

////////////////////////////////

class BayesVecKLGaussianVarComponent: public RandomComponent {
 public:
  void Init(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale, bool output_sum, bool has_output);
  explicit BayesVecKLGaussianVarComponent(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale, bool output_sum, bool has_output) {
    Init(input_dim, output_dim, scale, input_frame_scale, inv_frame_scale, output_sum, has_output);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kBackpropNeedsOutput|kUsesMemo|kRandomComponent;
  }
  BayesVecKLGaussianVarComponent(): input_dim_(0), output_dim_(0), KL_scale_(1.0), input_frame_scale_(false), inv_frame_scale_(false), rand_per_frame_(false), KL_output_(true) { }
  virtual std::string Type() const { return "BayesVecKLGaussianVarComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual std::string Info() const;
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new BayesVecKLGaussianVarComponent(input_dim_, output_dim_, KL_scale_, input_frame_scale_, inv_frame_scale_, rand_per_frame_, KL_output_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

  virtual void DeleteMemo(void *memo) const { delete static_cast<Memo*>(memo); }
  
 protected:
  
  struct Memo {
	CuVector<BaseFloat> rand_vec;
	CuMatrix<BaseFloat> rand_mat;
	CuMatrix<BaseFloat> inv_prior_var;
	CuMatrix<BaseFloat> diff;
	CuMatrix<BaseFloat> diff_power_2;
	CuMatrix<BaseFloat> rate_power_2;
	CuVector<BaseFloat> frame_scale;
  };
  
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat KL_scale_;
  bool input_frame_scale_;
  bool inv_frame_scale_;
  bool KL_output_;
  bool rand_per_frame_;
};



//////////////////////////

class InputVectorLinearComponent: public Component {
public:
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  void Init(const std::vector<int32> &sizes); // the vector is of the input dim
                                              // (>= 1) for each output dim.
  void Init(int32 input_dim, int32 output_dim);
  void GetSizes(std::vector<int32> *sizes) const; // Get a vector saying, for
                                                  // each output-dim, how many
                                                  // inputs were summed over.
  virtual void InitFromConfig(ConfigLine *cfl);
  InputVectorLinearComponent() { }
  virtual std::string Type() const { return "InputVectorLinearComponent"; }
  virtual int32 Properties() const { return kSimpleComponent|kBackpropNeedsInput|kBackpropNeedsOutput;; }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const;
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

private:
  KALDI_DISALLOW_COPY_AND_ASSIGN(InputVectorLinearComponent);
  // Note: Int32Pair is just struct{ int32 first; int32 second }; it's defined
  // in cu-matrixdim.h as extern "C" which is needed for the CUDA interface.
  CuArray<Int32Pair> indexes_; // for each output index, the (start, end) input
                               // index.
  CuArray<int32> reverse_indexes_; // for each input index, the output index.
  int32 input_dim_;
  int32 output_dim_;
};


/////////////////////////////////////////////////////

class BayesAffineComponent: public UpdatableComponent {
 public:
  virtual int32 InputDim() const { return linear_params_mean_.NumCols(); }
  virtual int32 OutputDim() const { return linear_params_mean_.NumRows(); }

  BaseFloat OrthonormalConstraint() const { return orthonormal_constraint_; }

  virtual std::string Info() const;
  virtual void InitFromConfig(ConfigLine *cfl);

  BayesAffineComponent(): orthonormal_constraint_(0.0), KL_scale_(0.00001), 
							test_mode_(false), share_std_input_(false), share_std_output_(false),
							share_std_input_sampling_(false), share_std_output_sampling_(false),
							use_exp_std_(false), update_prior_(false) { } // use Init to really initialize.
  virtual std::string Type() const { return "BayesAffineComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kUpdatableComponent|
        kBackpropNeedsInput|kBackpropAdds|kUsesMemo;
  }


  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual void DeleteMemo(void *memo) const {
    delete static_cast<CuMatrix<BaseFloat>*>(memo);
  }
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

  virtual Component* Copy() const;


  // Some functions from base-class UpdatableComponent.
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const;
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);

  // Some functions that are specific to this class.

  virtual void SetParams(const CuVectorBase<BaseFloat> &bias,
                         const CuMatrixBase<BaseFloat> &linear_mean,
						 const CuMatrixBase<BaseFloat> &linear_std,
						 const CuMatrixBase<BaseFloat> &linear_prior_mean,
						 const CuMatrixBase<BaseFloat> &linear_prior_std);
  const CuVector<BaseFloat> &BiasParams() const { return bias_params_; }
  CuVector<BaseFloat> &BiasParams() { return bias_params_; }
  const CuMatrix<BaseFloat> &LinearParams() const { return linear_params_mean_; }
  CuMatrix<BaseFloat> &LinearParams() { return linear_params_mean_; }
  explicit BayesAffineComponent(const BayesAffineComponent &other);
  // The next constructor is used in converting from nnet1.
  BayesAffineComponent(const CuMatrixBase<BaseFloat> &linear_params_mean,
					const CuMatrixBase<BaseFloat> &linear_params_std,
					const CuMatrixBase<BaseFloat> &linear_prior_mean,
					const CuMatrixBase<BaseFloat> &linear_prior_std,
                  const CuVectorBase<BaseFloat> &bias_params,
                  BaseFloat learning_rate);
  // This function resizes the dimensions of the component, setting the
  // parameters to zero, while leaving any other configuration values the same.
  virtual void Resize(int32 input_dim, int32 output_dim);

  void Init(int32 input_dim, int32 output_dim,
            BaseFloat param_mean_stddev, BaseFloat param_std_stddev, BaseFloat bias_stddev, BaseFloat prior_mean, BaseFloat prior_std);
 protected:
  void Init(std::string matrix_filename);

  friend class NaturalGradientBayesAffineComponent;
  // This function Update() is for extensibility; child classes may override
  // this, e.g. for natural gradient update.
  virtual void Update(
      const std::string &debug_info,
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv,
	  const CuMatrixBase<BaseFloat> &rand_mat) {
    UpdateSimple(in_value, out_deriv, rand_mat);
  }
  // UpdateSimple is used when *this is a gradient.  Child classes may override
  // this if needed, but typically won't need to.
  virtual void UpdateSimple(
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv,
	  const CuMatrixBase<BaseFloat> &rand_mat);

  const BayesAffineComponent &operator = (const BayesAffineComponent &other); // Disallow.
  CuMatrix<BaseFloat> linear_params_mean_;
  CuMatrix<BaseFloat> linear_params_std_;
  CuMatrix<BaseFloat> linear_prior_mean_;
  CuMatrix<BaseFloat> linear_prior_std_;
  CuVector<BaseFloat> bias_params_;
  // see documentation at the top of this class for more information on the
  // following.
  BaseFloat orthonormal_constraint_;
  BaseFloat KL_scale_;
  
  bool test_mode_;
  bool share_std_input_;
  bool share_std_output_;
  bool share_std_input_sampling_;
  bool share_std_output_sampling_;
  bool use_exp_std_;
  bool update_prior_;
  
  //CuMatrix<BaseFloat> rand_mat_;
  //CuMatrix<BaseFloat> linear_params_;
};

////////////////////////////////////////////////////////////////////////////////////

class NaturalGradientBayesAffineComponent: public BayesAffineComponent {
 public:
  virtual std::string Type() const { return "NaturalGradientBayesAffineComponent"; }
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  // this constructor does not really initialize, use InitFromConfig() or Read().
  NaturalGradientBayesAffineComponent() { }
  void InitFromConfig(ConfigLine *cfl);
  virtual std::string Info() const;
  virtual Component* Copy() const;
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void FreezeNaturalGradient(bool freeze);

  virtual void ConsolidateMemory();

  // copy constructor
  explicit NaturalGradientBayesAffineComponent(
      const NaturalGradientBayesAffineComponent &other);
  NaturalGradientBayesAffineComponent(
      const CuMatrixBase<BaseFloat> &linear_params_mean,
		const CuMatrixBase<BaseFloat> &linear_params_std,
		const CuMatrixBase<BaseFloat> &linear_prior_mean,
		const CuMatrixBase<BaseFloat> &linear_prior_std,
        const CuVectorBase<BaseFloat> &bias_params);
 private:
  // disallow assignment operator.
  NaturalGradientBayesAffineComponent &operator= (
      const NaturalGradientBayesAffineComponent&);

  OnlineNaturalGradient preconditioner_in_;

  OnlineNaturalGradient preconditioner_out_;

  virtual void Update(
      const std::string &debug_info,
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv,
	  const CuMatrixBase<BaseFloat> &rand_mat);
};

////////////////////////////////////////////////////////////////////////////////////


/*
  LinearSelectColComponent represents a linear (matrix) transformation of its input, with
  a matrix as its trainable parameters.  It's the same as
  NaturalGradientAffineComponent, but without the bias term.

  Configuration values accepted by this component:

  Values inherited from UpdatableComponent (see its declaration in
  nnet-component-itf for details):
     learning-rate
     learning-rate-factor
     max-change

  Values used in initializing the component's parameters:
     input-dim             =1.  The input dimension.
     output-dim            e.g. output-dim=1024.  The output dimension.
	 col-num				e.g. input-dim=1024. The col dimension.
     param-stddev          e.g. param-stddev=0.025.  The standard deviation
                           used to randomly initialize the linear parameters
                           (as Gaussian random values * param-stddev).
                           Defaults to 1/sqrt(input-dim), which is Glorot
                           initialization.
     matrix                e.g. matrix=foo/bar/init.mat  May be used as an
                           alternative to (input-dim, output-dim, param-stddev,
                           bias-stddev, bias-mean) to initialize the parameters.
                           Dimension is output-dim by (input-dim + 1), last
                           column is interpreted as the bias.
    orthonormal-constraint=0.0   If you set this to 1.0, then
                           the linear_params_ matrix will be (approximately)
                           constrained during training to have orthonormal rows
                           (or columns, whichever is fewer).. it turns out the
                           real name for this is a "semi-orthogonal" matrix.
                           You can choose a positive nonzero value different
                           than 1.0 to have a scaled semi-orthgonal matrix,
                           i.e. with singular values at the selected value
                           (e.g. 0.5, or 2.0).  This is not enforced inside the
                           component itself; you have to call
                           ConstrainOrthonormal() from the training code to do
                           this.  All this component does is return the
                           OrthonormalConstraint() value.  If you set this to a
                           negative value, it's like saying "for any value",
                           i.e. it will constrain the parameter matrix to be
                           closer to "any alpha" times a semi-orthogonal matrix,
                           without changing its overall norm.

   Options to the natural gradient (you won't normally have to set these,
   the defaults are suitable):

      use-natural-gradient=true   Set this to false to disable the natural-gradient
                            update entirely (it will do regular SGD).
      num-samples-history   Number of frames used as the time-constant to
                            determine how 'up-to-date' the Fisher-matrix
                            estimates are.  Smaller -> more up-to-date, but more
                            noisy.  default=2000.
      alpha                 Constant that determines how much we smooth the
                            Fisher-matrix estimates with the unit matrix.
                            Larger means more smoothing. default=4.0
      rank-in               Rank used in low-rank-plus-unit estimate of Fisher
                            matrix in the input space.  default=20.
      rank-out              Rank used in low-rank-plus-unit estimate of Fisher
                            matrix in the output-derivative space.  default=80.
      update-period         Determines after with what frequency (in
                            minibatches) we update the Fisher-matrix estimates;
                            making this > 1 saves a little time in training.
                            default=4.
*/
class LinearSelectColComponent: public UpdatableComponent {
 public:
  virtual int32 InputDim() const { return 1; }
  virtual int32 OutputDim() const { return params_.NumRows(); }

  virtual std::string Type() const { return "LinearSelectColComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kUpdatableComponent|
        kPropagateAdds|kBackpropAdds|kUsesMemo;
  }

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, // in_value
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  // this constructor does not really initialize, use InitFromConfig() or Read().
  LinearSelectColComponent() { }
  void InitFromConfig(ConfigLine *cfl);
  virtual std::string Info() const;
  virtual Component* Copy() const;
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const;
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);
  virtual void FreezeNaturalGradient(bool freeze);
  virtual void ConsolidateMemory();

  virtual void DeleteMemo(void *memo) const {
    delete static_cast<CuMatrix<BaseFloat>*>(memo);
  }
  
  // copy constructor
  explicit LinearSelectColComponent(const LinearSelectColComponent &other);

  explicit LinearSelectColComponent(const CuMatrix<BaseFloat> &params);

  BaseFloat OrthonormalConstraint() const { return orthonormal_constraint_; }
  CuMatrixBase<BaseFloat> &Params() { return params_; }
  const CuMatrixBase<BaseFloat> &Params() const { return params_; }
 private:

  // disallow assignment operator.
  LinearSelectColComponent &operator= (
      const LinearSelectColComponent&);

  CuMatrix<BaseFloat> params_;

  BaseFloat orthonormal_constraint_;
  // If true (and if no this->is_gradient_), use natural gradient updates.
  bool use_natural_gradient_;
  OnlineNaturalGradient preconditioner_in_;
  OnlineNaturalGradient preconditioner_out_;
};

////////////////////////////////

class VecL2NormComponent: public Component {
 public:
  void Init(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale);
  explicit VecL2NormComponent(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale) {
    Init(input_dim, output_dim, scale, input_frame_scale, inv_frame_scale);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kPropagateInPlace;
  }
  VecL2NormComponent(): input_dim_(0), output_dim_(0), scale_(1.0), input_frame_scale_(false), inv_frame_scale_(false) { }
  virtual std::string Type() const { return "VecL2NormComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual std::string Info() const;
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new VecL2NormComponent(input_dim_, output_dim_, scale_, input_frame_scale_, inv_frame_scale_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

 protected:
  
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  bool input_frame_scale_;
  bool inv_frame_scale_;
};

////////////////////////////////

class VeclogNormComponent: public Component {
 public:
  void Init(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale);
  explicit VeclogNormComponent(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale) {
    Init(input_dim, output_dim, scale, input_frame_scale, inv_frame_scale);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kPropagateInPlace;
  }
  VeclogNormComponent(): input_dim_(0), output_dim_(0), scale_(1.0), input_frame_scale_(false), inv_frame_scale_(false) { }
  virtual std::string Type() const { return "VeclogNormComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual std::string Info() const;
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new VeclogNormComponent(input_dim_, output_dim_, scale_, input_frame_scale_, inv_frame_scale_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

 protected:
  
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  bool input_frame_scale_;
  bool inv_frame_scale_;
};

/////////////////////////////

class PnormSumBlockComponent: public Component {
 public:
  explicit PnormSumBlockComponent(const PnormSumBlockComponent &other);
  PnormSumBlockComponent() { }
  virtual std::string Type() const { return "PnormSumBlockComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateAdds|kBackpropAdds|kBackpropNeedsInput|kBackpropNeedsOutput;
  }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new PnormSumBlockComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  PnormSumBlockComponent &operator = (const PnormSumBlockComponent &other); // Disallow.
};

/////////////////////////////

class WeightSumBlockComponent: public Component {
 public:
  explicit WeightSumBlockComponent(const WeightSumBlockComponent &other);
  WeightSumBlockComponent() { }
  virtual std::string Type() const { return "WeightSumBlockComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateAdds|kBackpropAdds|kBackpropNeedsInput|kBackpropNeedsOutput;
  }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new WeightSumBlockComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  BaseFloat weight_deriv_scale_;
  int32 mix_num_;
  WeightSumBlockComponent &operator = (const WeightSumBlockComponent &other); // Disallow.
};

/////////////////////////////

class WeightSumBlockFixComponent: public Component {
 public:
  explicit WeightSumBlockFixComponent(const WeightSumBlockFixComponent &other);
  WeightSumBlockFixComponent() { }
  virtual std::string Type() const { return "WeightSumBlockFixComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateAdds|kBackpropAdds|kBackpropNeedsInput|kBackpropNeedsOutput;
  }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new WeightSumBlockFixComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  BaseFloat weight_deriv_scale_;
  int32 mix_num_;
  WeightSumBlockFixComponent &operator = (const WeightSumBlockFixComponent &other); // Disallow.
};

/////////////////////////////

class WeightSumBlockRandomComponent: public RandomComponent {
 public:
  explicit WeightSumBlockRandomComponent(const WeightSumBlockRandomComponent &other);
  WeightSumBlockRandomComponent() { }
  virtual std::string Type() const { return "WeightSumBlockRandomComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateAdds|kBackpropAdds|kBackpropNeedsInput|kBackpropNeedsOutput|kRandomComponent|kUsesMemo;
  }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual void DeleteMemo(void *memo) const { delete static_cast<Memo*>(memo); }
  virtual std::string Info() const;
  virtual Component* Copy() const { return new WeightSumBlockRandomComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  struct Memo {
	CuMatrix<BaseFloat> rand_mask;
  };
  
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  BaseFloat weight_deriv_scale_;
  int32 mix_num_;
  int32 rand_num_;
  
  bool rand_per_frame_;
  bool test_time_sample_;
  
  WeightSumBlockRandomComponent &operator = (const WeightSumBlockRandomComponent &other); // Disallow.
};

//////////

class SoftmaxKLComponent: public Component {
 public:
  void Init(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale);
  explicit SoftmaxKLComponent(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale) {
    Init(input_dim, output_dim, scale, input_frame_scale, inv_frame_scale);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsOutput|kBackpropNeedsInput;
  }
  SoftmaxKLComponent(): input_dim_(0), output_dim_(0), scale_(1.0), input_frame_scale_(false), inv_frame_scale_(false) { }
  virtual std::string Type() const { return "SoftmaxKLComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual std::string Info() const;
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new SoftmaxKLComponent(input_dim_, output_dim_, scale_, input_frame_scale_, inv_frame_scale_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

 protected:
  
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  bool input_frame_scale_;
  bool inv_frame_scale_;
};

//////////

class SumOneKLComponent: public Component {
 public:
  void Init(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale);
  explicit SumOneKLComponent(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale) {
    Init(input_dim, output_dim, scale, input_frame_scale, inv_frame_scale);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsOutput|kBackpropNeedsInput;
  }
  SumOneKLComponent(): input_dim_(0), output_dim_(0), scale_(1.0), input_frame_scale_(false), inv_frame_scale_(false) { }
  virtual std::string Type() const { return "SumOneKLComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual std::string Info() const;
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new SumOneKLComponent(input_dim_, output_dim_, scale_, input_frame_scale_, inv_frame_scale_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

 protected:
  
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  bool input_frame_scale_;
  bool inv_frame_scale_;
};

//////////

class BLHUCGMMComponent: public UpdatableComponent {
 public:
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }

  virtual std::string Type() const { return "BLHUCGMMComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kUpdatableComponent|kBackpropNeedsInput|kBackpropNeedsOutput|kUsesMemo|
        kPropagateAdds|kBackpropAdds|kRandomComponent;
  }

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  // this constructor does not really initialize, use InitFromConfig() or Read().
  BLHUCGMMComponent() { }
  void InitFromConfig(ConfigLine *cfl);
  virtual std::string Info() const;
  virtual Component* Copy() const;
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const;
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);
  virtual void FreezeNaturalGradient(bool freeze);
  virtual void ConsolidateMemory();
  void SetTestMode(bool test_mode) { test_mode_ = test_mode; }
  virtual void DeleteMemo(void *memo) const { delete static_cast<Memo*>(memo); }
  
  // copy constructor
  explicit BLHUCGMMComponent(const BLHUCGMMComponent &other);

  explicit BLHUCGMMComponent(const CuMatrix<BaseFloat> &params, BaseFloat KL_scale, 
	BaseFloat mean_update_scale, BaseFloat std_update_scale, BaseFloat prior_mean_update_scale, BaseFloat prior_std_update_scale,
	bool test_mode, bool share_std_dim, bool share_mix_sampling, bool input_frame_scale, bool inv_frame_scale,
	int32 activation, int32 mix_num, int32 input_dim, int32 output_dim);

  BaseFloat OrthonormalConstraint() const { return orthonormal_constraint_; }
  CuMatrixBase<BaseFloat> &Params() { return params_; }
  const CuMatrixBase<BaseFloat> &Params() const { return params_; }
 private:

  // disallow assignment operator.
  BLHUCGMMComponent &operator= (
      const BLHUCGMMComponent&);

  CuMatrix<BaseFloat> params_;
  
  CuRand<BaseFloat> random_generator_;
  
  BaseFloat KL_scale_;
  BaseFloat mean_update_scale_;
  BaseFloat std_update_scale_;
  BaseFloat prior_mean_update_scale_;
  BaseFloat prior_std_update_scale_;
  bool test_mode_;
  bool share_std_dim_;
  bool share_mix_sampling_;
  bool input_frame_scale_;
  bool inv_frame_scale_;
  int32 activation_; // sigmoid=0, relu=1, exp=2, lin=3
  int32 mix_num_;
  
  int32 input_dim_;
  int32 output_dim_;

  struct Memo {
	CuVector<BaseFloat> rand_vec;
	CuMatrix<BaseFloat> rand_mat;
	CuMatrix<BaseFloat> LHUC_out;
  };
  
  BaseFloat orthonormal_constraint_;
  // If true (and if no this->is_gradient_), use natural gradient updates.
  bool use_natural_gradient_;
  OnlineNaturalGradient preconditioner_in_;
  OnlineNaturalGradient preconditioner_out_;
};


//////////

class KLAdaptComponent: public Component {
 public:
  explicit KLAdaptComponent(const KLAdaptComponent &other):
      dim_(other.dim_), SI_scale_(other.SI_scale_) { }
  KLAdaptComponent() { }
  virtual std::string Type() const { return "KLAdaptComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput;
  }
  virtual int32 InputDim() const { return 2*dim_; }
  virtual int32 OutputDim() const { return dim_; }
  virtual Component *Copy() { return new KLAdaptComponent(*this); }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new KLAdaptComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 dim_;
  BaseFloat SI_scale_;

  KLAdaptComponent &operator = (const KLAdaptComponent &other); // Disallow.
};


/////////////////////////////

class GumbelSoftmaxComponent : public RandomComponent {
 public:
  void Init(int32 dim, BaseFloat temperature = 1.0,
            bool apply_log = true, int32 temperature_decrease=0, BaseFloat temperature_decrease_proportion = 1.0, BaseFloat temperature_decrease_minus = 0.0);

  GumbelSoftmaxComponent(int32 dim, BaseFloat temperature = 1.0,
                   bool apply_log = true, int32 temperature_decrease=0, BaseFloat temperature_decrease_proportion = 1.0, BaseFloat temperature_decrease_minus = 0.0) {
    Init(dim, temperature, apply_log);
  }

  GumbelSoftmaxComponent(): dim_(0), temperature_(1.0),
                      apply_log_(true), temperature_decrease_(0), temperature_decrease_proportion_(1.0), temperature_decrease_minus_(0.0) { }

  GumbelSoftmaxComponent(const GumbelSoftmaxComponent &other);

  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|
        kBackpropNeedsOutput|kRandomComponent;
  }
  virtual std::string Type() const { return "GumbelSoftmaxComponent"; }

  virtual void InitFromConfig(ConfigLine *cfl);

  virtual int32 InputDim() const { return dim_; }

  virtual int32 OutputDim() const { return dim_; }

  virtual void Read(std::istream &is, bool binary);

  // Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual Component* Copy() const;

  virtual std::string Info() const;

  void SetTemperature(BaseFloat temperature) {
    temperature_ = temperature;
  }

  BaseFloat Temperature() const { return temperature_; }
 private:
  int32 dim_;
  /// dropout-proportion is the proportion that is dropped out,
  /// e.g. if 0.1, we set 10% to zero value.
  BaseFloat temperature_;
  bool apply_log_;
  
  int32 temperature_decrease_;
  BaseFloat temperature_decrease_proportion_;
  BaseFloat temperature_decrease_minus_;
};

//////////

class SoftmaxKLPriorComponent: public Component {
 public:
  void Init(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale);
  explicit SoftmaxKLPriorComponent(int32 input_dim, int32 output_dim, BaseFloat scale, bool input_frame_scale, bool inv_frame_scale) {
    Init(input_dim, output_dim, scale, input_frame_scale, inv_frame_scale);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsOutput|kBackpropNeedsInput;
  }
  SoftmaxKLPriorComponent(): input_dim_(0), output_dim_(0), scale_(1.0), input_frame_scale_(false), inv_frame_scale_(false) { }
  virtual std::string Type() const { return "SoftmaxKLPriorComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual std::string Info() const;
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new SoftmaxKLPriorComponent(input_dim_, output_dim_, scale_, input_frame_scale_, inv_frame_scale_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

 protected:
  
  int32 input_dim_;
  int32 output_dim_;
  BaseFloat scale_;
  bool input_frame_scale_;
  bool inv_frame_scale_;
};

////////////////////////////////

class PriorSelectKLGaussianComponent: public UpdatableComponent {
 public:
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }

  BaseFloat OrthonormalConstraint() const { return orthonormal_constraint_; }

  virtual std::string Info() const;
  virtual void InitFromConfig(ConfigLine *cfl);

  PriorSelectKLGaussianComponent(): orthonormal_constraint_(0.0), mix_num_(1), input_mix_num_(1), output_weight_(false), input_dim_(0), output_dim_(0) { } // use Init to really initialize.
  virtual std::string Type() const { return "PriorSelectKLGaussianComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kUpdatableComponent|
        kBackpropNeedsInput|kBackpropAdds|kUsesMemo;
  }


  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &, // out_value
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;

  virtual Component* Copy() const;
  virtual void DeleteMemo(void *memo) const { delete static_cast<Memo*>(memo); }

  // Some functions from base-class UpdatableComponent.
  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);
  virtual void PerturbParams(BaseFloat stddev);
  virtual BaseFloat DotProduct(const UpdatableComponent &other) const;
  virtual int32 NumParameters() const;
  virtual void Vectorize(VectorBase<BaseFloat> *params) const;
  virtual void UnVectorize(const VectorBase<BaseFloat> &params);

  // Some functions that are specific to this class.

  virtual void SetParams(const CuVectorBase<BaseFloat> &bias,
                         const CuMatrixBase<BaseFloat> &linear);
  const CuVector<BaseFloat> &BiasParams() const { return bias_params_; }
  CuVector<BaseFloat> &BiasParams() { return bias_params_; }
  const CuMatrix<BaseFloat> &LinearParams() const { return linear_params_; }
  CuMatrix<BaseFloat> &LinearParams() { return linear_params_; }
  explicit PriorSelectKLGaussianComponent(const PriorSelectKLGaussianComponent &other);
  // The next constructor is used in converting from nnet1.
  PriorSelectKLGaussianComponent(const CuMatrixBase<BaseFloat> &linear_params,
                  const CuVectorBase<BaseFloat> &bias_params,
                  BaseFloat learning_rate);
  // This function resizes the dimensions of the component, setting the
  // parameters to zero, while leaving any other configuration values the same.
  virtual void Resize(int32 input_dim, int32 output_dim);

  void Init(int32 input_dim, int32 output_dim,
            BaseFloat param_stddev, BaseFloat bias_stddev, BaseFloat mean_init, BaseFloat std_init);
 protected:
  void Init(std::string matrix_filename);

  friend class NaturalGradientPriorSelectKLGaussianComponent;
  // This function Update() is for extensibility; child classes may override
  // this, e.g. for natural gradient update.
  virtual void Update(
      const std::string &debug_info,
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv) {
    UpdateSimple(in_value, out_deriv);
  }
  // UpdateSimple is used when *this is a gradient.  Child classes may override
  // this if needed, but typically won't need to.
  virtual void UpdateSimple(
      const CuMatrixBase<BaseFloat> &in_value,
      const CuMatrixBase<BaseFloat> &out_deriv);

  const PriorSelectKLGaussianComponent &operator = (const PriorSelectKLGaussianComponent &other); // Disallow.
  CuMatrix<BaseFloat> linear_params_;
  CuVector<BaseFloat> bias_params_;
  // see documentation at the top of this class for more information on the
  // following.
  BaseFloat orthonormal_constraint_;
  
  int32 mix_num_;
  int32 input_mix_num_;
  bool output_weight_;
  int32 input_dim_;
  int32 output_dim_;
  
  struct Memo {
	CuMatrix<BaseFloat> select_id;
  };
};

///////////////////////////////

class InvertComponent: public NonlinearComponent {
 public:
  explicit InvertComponent(const InvertComponent &other): NonlinearComponent(other) { }
  InvertComponent() { }
  virtual std::string Type() const { return "InvertComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsOutput|kPropagateInPlace|kStoresStats;
  }
  virtual Component* Copy() const { return new InvertComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual void StoreStats(const CuMatrixBase<BaseFloat> &in_value,
                          const CuMatrixBase<BaseFloat> &out_value,
                          void *memo);
 private:
  // this function is called from Backprop code and only does something if the
  // self-repair-scale config value is set.
  void RepairGradients(const CuMatrixBase<BaseFloat> &out_value,
                       CuMatrixBase<BaseFloat> *in_deriv,
                       InvertComponent *to_update) const;

  InvertComponent &operator = (const InvertComponent &other); // Disallow.
};

////////////////////////////////////////////////////////


class SpecMaskOnlineComponent: public Component {
 public:
  explicit SpecMaskOnlineComponent(const SpecMaskOnlineComponent &other):
      dim_(other.dim_), width_filter_max_(other.width_filter_max_), width_time_max_(other.width_time_max_), rate_time_max_(other.rate_time_max_), num_filter_mask_(other.num_filter_mask_), num_time_mask_(other.num_time_mask_), mask_value_(other.mask_value_), mask_prob_(other.mask_prob_), mask_filter_(other.mask_filter_), mask_time_(other.mask_time_), mask_as_mean_(other.mask_as_mean_) { }
  SpecMaskOnlineComponent() { }
  virtual std::string Type() const { return "SpecMaskOnlineComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kPropagateInPlace|kBackpropInPlace;
  }
  virtual int32 InputDim() const { return dim_; }
  virtual int32 OutputDim() const { return dim_; }
  virtual Component *Copy() { return new SpecMaskOnlineComponent(*this); }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new SpecMaskOnlineComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 dim_;
  int32 width_filter_max_;
  int32 width_time_max_;
  BaseFloat rate_time_max_;
  int32 num_filter_mask_;
  int32 num_time_mask_;
  BaseFloat mask_value_;
  BaseFloat mask_prob_;
  bool mask_filter_;
  bool mask_time_;
  bool mask_as_mean_;

  SpecMaskOnlineComponent &operator = (const SpecMaskOnlineComponent &other); // Disallow.
};

///////////////////////////////////////////////////////////////////////

class FramewiseLinearComponent: public Component {
 public:
  void Init(int32 input_dim, int32 output_dim, int32 feat_dim);
  explicit FramewiseLinearComponent(int32 input_dim, int32 output_dim, int32 feat_dim) {
    Init(input_dim, output_dim, feat_dim);
  }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kPropagateAdds|kBackpropAdds;
  }
  FramewiseLinearComponent(): input_dim_(0), output_dim_(0), feat_dim_(0) { }
  virtual std::string Type() const { return "FramewiseLinearComponent"; }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
  virtual Component* Copy() const { return new FramewiseLinearComponent(input_dim_,
                                                              output_dim_,feat_dim_); }

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;

 protected:
  int32 input_dim_; // input = [ feat(T*N) param(T*(N*M)) ]
  int32 output_dim_; // output(T*M)
  int32 feat_dim_; // feat(N)
};


///////////////////////////////////////////////////////////////////////////////

class MeanAllMemoComponent: public Component {
 public:

  MeanAllMemoComponent() { }

  // call this with 'true' to set 'test mode' where the batch normalization is
  // done with stored stats.  There won't normally be any need to specially
  // accumulate these stats; they are stored as a matter of course on each
  // iteration of training, as for NonlinearComponents, and we'll use the stats
  // from the most recent [script-level] iteration.
  // (Note: it will refuse to actually set test-mode to true if there
  // are no stats stored.)
  void SetTestMode(bool test_mode);

  // constructor using another component
  MeanAllMemoComponent(const MeanAllMemoComponent &other);

  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return output_dim_; }

  virtual std::string Info() const;
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual std::string Type() const { return "MeanAllMemoComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kBackpropNeedsInput|kUsesMemo;
  }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                         const CuMatrixBase<BaseFloat> &in,
                         CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &in_value,
                        const CuMatrixBase<BaseFloat> &out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;

  virtual void Read(std::istream &is, bool binary); // This Read function
  // requires that the Component has the correct type.

  /// Write component to stream
  virtual void Write(std::ostream &os, bool binary) const;
  virtual Component* Copy() const { return new MeanAllMemoComponent(*this); }

  virtual void Scale(BaseFloat scale);
  virtual void Add(BaseFloat alpha, const Component &other);

  virtual void DeleteMemo(void *memo) const { delete static_cast<Memo*>(memo); }


 private:

  struct Memo {
    CuVector<BaseFloat> count;
    CuMatrix<BaseFloat> stats_sum;
    CuMatrix<BaseFloat> stats_sumsq;
    CuMatrix<BaseFloat> feat_mean;
    CuMatrix<BaseFloat> feat_std;
  };

  void Check() const;
  
  void ComputeStats(void *memo);

  // Dimension of the input and output.
  int32 input_dim_;
  int32 output_dim_;

  // Used to avoid exact-zero variances, epsilon has the dimension of a
  // covariance.
  BaseFloat epsilon_;
  
  BaseFloat backprop_scale_;
  BaseFloat mem_decay_rate_;

  bool test_mode_;
  bool output_std_;

  int32 spk_num_;

  // total count of stats stored by StoreStats().
  CuVector<BaseFloat> count_;
  // sum-of-data component of stats of input data.
  CuMatrix<BaseFloat> stats_sum_;
  // sum-of-squared component of stats of input data.
  CuMatrix<BaseFloat> stats_sumsq_;
  
  MeanAllMemoComponent &operator = (const MeanAllMemoComponent &other); // Disallow.
};

//////////

/**
   Id2OnehotComponent just samples from a std normal dist, omitting the input:

      dim               E.g. dim=1024.  Required.
*/
class Id2OnehotComponent: public Component {
 public:
  void Init(int32 dim, int32 input_dim);
  Id2OnehotComponent(int32 dim, int32 input_dim) {
    Init(dim, input_dim);
  }
  Id2OnehotComponent(): dim_(0), input_dim_(0) { }
  explicit Id2OnehotComponent(const Id2OnehotComponent &other):
      dim_(other.dim_), input_dim_(other.input_dim_) { }
  virtual std::string Type() const { return "Id2OnehotComponent"; }
  virtual int32 Properties() const {
    return kSimpleComponent|kRandomComponent;
  }
  virtual int32 InputDim() const { return input_dim_; }
  virtual int32 OutputDim() const { return dim_; }
  virtual Component *Copy() { return new Id2OnehotComponent(*this); }
  virtual void InitFromConfig(ConfigLine *cfl);
  virtual void Read(std::istream &is, bool binary);
  virtual void Write(std::ostream &os, bool binary) const;
  virtual std::string Info() const;
  virtual Component* Copy() const { return new Id2OnehotComponent(*this); }
  virtual void* Propagate(const ComponentPrecomputedIndexes *indexes,
                          const CuMatrixBase<BaseFloat> &in,
                          CuMatrixBase<BaseFloat> *out) const;
  virtual void Backprop(const std::string &debug_info,
                        const ComponentPrecomputedIndexes *indexes,
                        const CuMatrixBase<BaseFloat> &, //in_value
                        const CuMatrixBase<BaseFloat> &, // out_value,
                        const CuMatrixBase<BaseFloat> &out_deriv,
                        void *memo,
                        Component *to_update,
                        CuMatrixBase<BaseFloat> *in_deriv) const;
 private:
  int32 dim_;
  int32 input_dim_;
  
  Id2OnehotComponent &operator = (const Id2OnehotComponent &other); // Disallow.
};


} // namespace nnet3
} // namespace kaldi


#endif
