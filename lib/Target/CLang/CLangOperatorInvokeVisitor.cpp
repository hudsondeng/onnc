#include "CLangOperatorInvokeVisitor.h"

#include <onnc/ADT/StringRef.h>
#include <onnc/IR/ComputeOperator.h>
#include <onnc/IR/Compute.h>

#include <initializer_list>
#include <iostream>
#include <string>
#include <sstream>
#include <type_traits>

#define PP_GEN_CLASS_NAME() CLangOperatorInvokeVisitor
#define PP_GEN_VISIT_DEF(type, param) \
  PP_GEN_VISIT_RETURN_TYPE() PP_GEN_CLASS_NAME()::visit(PP_GEN_VISIT_PARAM_TYPE(type) param) { \
    auto&& target = internal::getTarget(PP_STRINGIFY(type)); \
    stream << indent_ << "// " << target << '\n'; \
    PP_ENTER_SCOPE(stream, indent_); \
    visitImpl(param, indent_ + 1, target); \
    PP_LEAVE_SCOPE(stream, indent_); \
  } \
  PP_GEN_VISIT_RETURN_TYPE() PP_GEN_CLASS_NAME()::visitImpl(PP_GEN_VISIT_PARAM_TYPE(type) param, \
                                                            internal::Indent indent, const std::string& target)

#define PP_ENTER_SCOPE(stream, indent) stream << indent << "{\n"
#define PP_LEAVE_SCOPE(stream, indent) stream << indent << "}\n"

namespace onnc {
namespace internal {
  enum Type : unsigned
  {
    INT32, FLOAT, PTR_TO_VOID
  };

  using identifier_type = PP_GEN_CLASS_NAME()::identifier_type;
  using expression_type = PP_GEN_CLASS_NAME()::expression_type;

  static_assert(std::is_convertible<std::string, identifier_type>::value, "Should can create identifier_type from std::string");
  static_assert(std::is_convertible<std::string, expression_type>::value, "Should can create expression_type from std::string");
  static_assert(std::is_convertible<identifier_type, expression_type>::value, "identifier_type itself is a kind of expression_type");

  inline constexpr PP_GEN_CLASS_NAME()::stream_type& operator<<(PP_GEN_CLASS_NAME()::stream_type& stream, Type type)
  {
    switch (type) {
    case INT32:
      stream << "int32_t";
      break;
    case FLOAT:
      stream << "float";
      break;
    case PTR_TO_VOID:
      stream << "void*";
      break;
    default:
      assert(false && "meet unknown type");
    }
    return stream;
  }

  inline std::string getTarget(const char* op)
  {
    std::ostringstream stream;
    stream << "ONNC_RUNTIME_" << StringRef{op}.lower() << "_float";
    return stream.str();
  }

  inline identifier_type getIdentifier()
  {
    static unsigned serialNumber = 0;

    std::ostringstream stream;
    stream << "v" << ++serialNumber;
    return stream.str();
  }

  template <Type type, typename RandomAccessRange>
  inline identifier_type defineArray(PP_GEN_CLASS_NAME()::stream_type& stream, Indent indent, const RandomAccessRange& range)
  {
    identifier_type array = getIdentifier();
    stream << indent << type << " " << array << "[] = { ";
    bool isFirst = true;
    for (const auto& element : range) {
      if (!isFirst) {
        stream << ", ";
      }
      stream << element;
      isFirst = false;
    }
    stream << " };\n";
    return array;
  }

  inline CLangMemoryBlock::address_type getOffset(const CLangMeta& meta, const Tensor& tensor)
  {
    for (const auto& entry : meta.packedInternalMemoryBlocks) {
      const auto* const value  = entry.first;
      const auto&       memory = entry.second;
      if (value == &tensor) {
        return memory.offset;
      }
    }

    assert(false && "cannot find such tensor in meta");
    return 0;
  }

  template <Type type>
  inline expression_type cast(const expression_type& expr)
  {
    std::ostringstream stream;
    stream << "((" << type << ")" << expr << ")";
    return stream.str();
  }

  inline identifier_type defineDimensionArray(PP_GEN_CLASS_NAME()::stream_type& stream, Indent indent, const Tensor& tensor)
  {
    return defineArray<INT32>(stream, indent, tensor.getDimensions());
  }

  inline void invoke(PP_GEN_CLASS_NAME()::stream_type& stream, Indent indent, const identifier_type& name, std::initializer_list<expression_type> exprs)
  {
    stream << indent << name << "(";
    bool isFirst = true;
    for (const auto& expr : exprs) {
      if (!isFirst) {
        stream << ", ";
      }
      stream << expr;
      isFirst = false;
    }
    stream << ");\n";
  }

  inline expression_type getArraySize(const identifier_type& array)
  {
    std::ostringstream stream;
    stream << "(sizeof(" << array << ") / sizeof(*" << array << "))";
    return stream.str();
  }
} // namespace internal

using namespace internal;

void PP_GEN_CLASS_NAME()::visit(const Module& module)
{
  for (const ComputeOperator& computeOperator : *module.getRootComputeGraph()) {
    computeOperator.accept(*this);
  }
}

inline identifier_type PP_GEN_CLASS_NAME()::defineTensor(internal::Indent indent, const Tensor& tensor)
{
  identifier_type id = getIdentifier();
  stream << indent << PTR_TO_VOID << " const " << id
         << " = " << memory << " + " << getOffset(meta, tensor)
         << '\n';
}

#include "internal/Conv.inc"
#include "internal/Gemm.inc"
#include "internal/LRN.inc"
#include "internal/MaxPool.inc"
#include "internal/Relu.inc"
#include "internal/Reshape.inc"
#include "internal/Softmax.inc"

} // namespace onnc
