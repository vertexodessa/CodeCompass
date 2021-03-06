#ifndef CC_PARSER_PPINCLUDECALLBACK_H
#define CC_PARSER_PPINCLUDECALLBACK_H

#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/AST/ASTContext.h>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/cppheaderinclusion.h>
#include <model/cppheaderinclusion-odb.hxx>

#include <parser/parsercontext.h>

#include <util/logutil.h>

#include "manglednamecache.h"

namespace cc
{
namespace parser
{

class PPIncludeCallback : public clang::PPCallbacks
{
public:
  PPIncludeCallback(
    ParserContext& ctx_,
    clang::ASTContext& astContext_,
    MangledNameCache& mangledNameCache_,
    clang::Preprocessor& pp_);

  ~PPIncludeCallback();

  virtual void InclusionDirective(
    clang::SourceLocation HashLoc,
    const clang::Token &IncludeTok,
    clang::StringRef FileName,
    bool IsAngled,
    clang::CharSourceRange FilenameRange,
    const clang::FileEntry *File,
    clang::StringRef SearchPath,
    clang::StringRef RelativePath,
    const clang::Module *Imported) override;

private:
  /**
   * This function creates an AST Node from a file.
   */
  model::CppAstNodePtr createFileAstNode(
    const model::FilePtr& file_,
    const clang::SourceRange& srcRange_);

  template <typename Cont>
  void persistAll(Cont& cont_)
  {
    for (typename Cont::value_type& item : cont_)
    {
      try
      {
        _ctx.db->persist(*item);
      }
      catch (const odb::object_already_persistent& ex)
      {
        LOG(warning)
          << item->toString() << std::endl
          << ex.what() << std::endl
          << "AST nodes in this translation unit will be ignored!";
      }
      catch (const odb::database_exception& ex)
      {
        LOG(debug) << ex.what();
        // TODO: Error code should be checked and rethrow if it is not unique
        // constraint error. Error code may be database specific.
      }
    }
  }

  ParserContext& _ctx;
  const std::string _cppSourceType;
  const clang::SourceManager& _clangSrcMgr;
  FileLocUtil _fileLocUtil;
  MangledNameCache& _mangledNameCache;

  std::vector<model::CppAstNodePtr>         _astNodes;
  std::vector<model::CppHeaderInclusionPtr> _headerIncs;
};

} // parser
} // cc

#endif // CC_PARSER_PPINCLUDECALLBACK_H
