/*

    Copyright (c) <YEAR> <OWNER ORGANIZATION NAME>. All rights reserved.


    Developed by: <NAME OF DEVELOPMENT GROUP>
    <NAME OF INSTITUTION>
    <URL FOR DEVELOPMENT GROUP/INSTITUTION>

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal with the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

        Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimers.
        Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimers in the documentation and/or other materials provided with the distribution.
        Neither the names of <NAME OF DEVELOPMENT GROUP>, <NAME OF INSTITUTION>, nor the names of its contributors may be used to endorse or promote products derived from this Software without specific prior written permission.


    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
*/
#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/Signals.h"
#include <iostream>
#include <vector>
using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

Rewriter rewriter;
int numFunctions = 0;
static cl::OptionCategory MyToolCategory("my-tool options");

class ExampleVisitor : public RecursiveASTVisitor<ExampleVisitor> {

    private:

        ASTContext *astContext; // used for getting additional AST info

        bool isInMainFile(const SourceLocation& s) const
        {
                return rewriter.getSourceMgr().getFileID(s)==rewriter.getSourceMgr().getMainFileID();
        }



    public:

        explicit ExampleVisitor(CompilerInstance *CI): astContext(&(CI->getASTContext())) // initialize private members
        {
            rewriter.setSourceMgr(astContext->getSourceManager(), astContext->getLangOpts());
        }

        virtual bool VisitFunctionDecl(FunctionDecl *func)
        {
            //numFunctions++;
            //string funcName = func->getNameInfo().getName().getAsString();
            if(!isInMainFile(func->getLocStart()))
                return true ;
            numFunctions++;
            if(func->hasBody())
            {
                rewriter.ReplaceText(func->getBody()->getLocStart(),"{//Inside func body "+func->getNameInfo().getName().getAsString()+"\n");
                rewriter.ReplaceText(func->getBody()->getLocEnd(),"//Exiting func body "+func->getNameInfo().getName().getAsString()+"}\n");

            }
            return true;
         }

        virtual bool VisitIfStmt(IfStmt* ifst)
        {
           if(!isInMainFile(ifst->getLocStart()))
               return true ;

           /*SourceRange vdcl= ifst->getConditionVariableDeclStmt()->getSourceRange();
           SourceLocation vbeg(vdcl.getBegin());
           SourceLocation vend(vdcl.getEnd());
           string vstr;

           while(1)
           {
                if(vbeg==vend)
                    break;
                const char *csp=rewriter.getSourceMgr().getCharacterData(vbeg);
                cond+=*csp;
                vbeg=vbeg.getLocWithOffset(1);
           }*/


           SourceLocation sc=ifst->getCond()->getExprLoc();
           string cond;
           while(1)//exraction condition as string
           {
               const char *csp=rewriter.getSourceMgr().getCharacterData(sc);
               if(*csp==')' )
                   break;
               cond+=*csp;
               sc=sc.getLocWithOffset(1);
           }

           SourceLocation thene=ifst->getThen()->getLocEnd();
           while(1)//finding then end
           {
               const char *csp=rewriter.getSourceMgr().getCharacterData(thene);
               if(*csp=='\n' )
                   break;
               thene=thene.getLocWithOffset(1);
           }

           SourceLocation ifsl=ifst->getThen()->getLocStart();
           bool brackIf=false;
           while(1)//loop to detect whether if is bracketed
           {

                const char *csp=rewriter.getSourceMgr().getCharacterData(ifsl);

                if(*csp==';')
                    break;
                if(*csp=='{')
                {
                    brackIf=true;
                    break;
                }
                 ifsl=ifsl.getLocWithOffset(1);
           }

           //string ifsstr,ifestr;

           if(brackIf)
           {
               errs()<<"inside brack if\n";
               rewriter.ReplaceText(ifst->getThen()->getLocStart(),"{//Inside if body with condition "+cond+"\n");
               rewriter.ReplaceText(ifst->getThen()->getLocEnd(),"//Exiting if body with condition "+cond+"\n}");
           }
           else
           {
               errs()<<"inside non brack if\n";
               rewriter.InsertText(ifst->getThen()->getLocStart(),"{//Inside if body with condition "+cond+"\n");
               rewriter.InsertText(thene,"//Exiting if body with condition "+cond+"\n}");
           }


           //The if part is over . Let us play with else part . Every thing will be same over here .
           //I should write some better code instead of repeating patterns . But you know .


           Stmt* estmt = ifst->getElse();
           if(estmt==NULL)
            return true;

           SourceLocation elsl=estmt->getLocStart();
           bool brackEsl=false;
           while(1)//loop to detect whether else is bracketed
           {

                const char *csp=rewriter.getSourceMgr().getCharacterData(elsl);

                if(*csp==';')
                    break;
                if(*csp=='{')
                {
                    brackEsl=true;
                    break;
                }
                 elsl=elsl.getLocWithOffset(1);
           }

           SourceLocation elsle=estmt->getLocEnd();//else end
           while(1)//finding then end
           {
               const char *csp=rewriter.getSourceMgr().getCharacterData(elsle);
               if(*csp=='\n' )
                   break;
               elsle=elsle.getLocWithOffset(1);
           }



           if(brackEsl)
           {
               errs()<<"inside brack if\n";
               rewriter.ReplaceText(estmt->getLocStart(),"{//Inside else body with condition "+cond+"\n");
               rewriter.ReplaceText(estmt->getLocEnd(),"//Exiting else body with condition "+cond+"\n}");
               return true;
           }
           else
           {
               rewriter.InsertText(estmt->getLocStart(),"{//Inside else with condition "+cond+"\n");
               rewriter.InsertText(elsle,"//Exiting else with condition"+cond+"\n} ");
               return true;
           }

        }


        virtual bool VisitWhileStmt(WhileStmt* wstmt)
        {
            if(!isInMainFile(wstmt->getLocStart()))
               return true ;

           /*SourceRange vdcl= ifst->getConditionVariableDeclStmt()->getSourceRange();
           SourceLocation vbeg(vdcl.getBegin());
           SourceLocation vend(vdcl.getEnd());
           string vstr;

           while(1)
           {
                if(vbeg==vend)
                    break;
                const char *csp=rewriter.getSourceMgr().getCharacterData(vbeg);
                cond+=*csp;
                vbeg=vbeg.getLocWithOffset(1);
           }*/


           SourceLocation sc=wstmt->getCond()->getExprLoc();
           string cond;
           while(1)//exraction condition as string
           {
               const char *csp=rewriter.getSourceMgr().getCharacterData(sc);
               if(*csp==')' )
                   break;
               cond+=*csp;
               sc=sc.getLocWithOffset(1);
           }



           SourceLocation wsl=wstmt->getBody()->getLocStart();
           bool brackWhile=false;
           while(1)//loop to detect whether if is bracketed
           {

                const char *csp=rewriter.getSourceMgr().getCharacterData(wsl);

                if(*csp==';')
                    break;
                if(*csp=='{')
                {
                    brackWhile=true;
                    break;
                }
                 wsl=wsl.getLocWithOffset(1);
           }

           SourceLocation wesl=wstmt->getBody()->getLocEnd();
          // rewriter.InsertText(wstmt->getBody()->getLocEnd(),"^");
           while(1)//finding then end
           {
               const char *csp=rewriter.getSourceMgr().getCharacterData(wesl);
               if(*csp=='\n' )
                   break;
               wesl=wesl.getLocWithOffset(1);
           }
           //string ifsstr,ifestr;
           //rewriter.InsertText(wstmt->getBody()->getLocStart(),"^");
           if(brackWhile)
           {
               errs()<<"inside brack while\n";
               rewriter.ReplaceText(wstmt->getBody()->getLocStart(),"{//Inside while body with condition "+cond+"\n");
               rewriter.ReplaceText(wstmt->getBody()->getLocEnd(),"//Exiting if body with condition "+cond+"\n}");
           }
           else
           {
               errs()<<"inside non brack while\n";
               rewriter.InsertText(wstmt->getBody()->getLocStart(),"{//Inside while body with condition "+cond+"\n");
               rewriter.InsertText(wesl,"//Exiting while body with condition "+cond+"\n}");
           }
           return true;
        }

        bool VisitForStmt(ForStmt* fstmt)
        {
            if(!isInMainFile(fstmt->getLocStart()))
               return true ;

           /*SourceRange vdcl= ifst->getConditionVariableDeclStmt()->getSourceRange();
           SourceLocation vbeg(vdcl.getBegin());
           SourceLocation vend(vdcl.getEnd());
           string vstr;

           while(1)
           {
                if(vbeg==vend)
                    break;
                const char *csp=rewriter.getSourceMgr().getCharacterData(vbeg);
                cond+=*csp;
                vbeg=vbeg.getLocWithOffset(1);
           }*/


           SourceLocation sc=fstmt->getCond()->getExprLoc();
           string cond;
           while(1)//exraction condition as string
           {
               const char *csp=rewriter.getSourceMgr().getCharacterData(sc);
               if(*csp==')' )
                   break;
               cond+=*csp;
               sc=sc.getLocWithOffset(1);
           }



           SourceLocation fsl=fstmt->getBody()->getLocStart();
           bool brackWhile=false;
           while(1)//loop to detect whether for is bracketed
           {

                const char *csp=rewriter.getSourceMgr().getCharacterData(fsl);

                if(*csp==';')
                    break;
                if(*csp=='{')
                {
                    brackWhile=true;
                    break;
                }
                 fsl=fsl.getLocWithOffset(1);
           }

           SourceLocation fesl=fstmt->getBody()->getLocEnd();
          // rewriter.InsertText(wstmt->getBody()->getLocEnd(),"^");
           while(1)//finding then end
           {
               const char *csp=rewriter.getSourceMgr().getCharacterData(fesl);
               if(*csp=='\n' )
                   break;
               fesl=fesl.getLocWithOffset(1);
           }
           //string ifsstr,ifestr;
           //rewriter.InsertText(wstmt->getBody()->getLocStart(),"^");
           if(brackWhile)
           {
               errs()<<"inside brack for\n";
               rewriter.ReplaceText(fstmt->getBody()->getLocStart(),"{//Inside while body with condition "+cond+"\n");
               rewriter.ReplaceText(fstmt->getBody()->getLocEnd(),"//Exiting if body with condition "+cond+"\n}");
           }
           else
           {
               errs()<<"inside non brack for\n";
               rewriter.InsertText(fstmt->getBody()->getLocStart(),"{//Inside while body with condition "+cond+"\n");
               rewriter.InsertText(fesl,"//Exiting while body with condition "+cond+"\n}");
           }
           return true;
        }

        virtual bool VisitDoStmt(DoStmt* wstmt)
        {
            if(!isInMainFile(wstmt->getLocStart()))
               return true ;



           SourceLocation sc=wstmt->getCond()->getExprLoc();
           string cond;
           while(1)//exraction condition as string
           {
               const char *csp=rewriter.getSourceMgr().getCharacterData(sc);
               if(*csp==')' )
                   break;
               cond+=*csp;
               sc=sc.getLocWithOffset(1);
           }



           SourceLocation wsl=wstmt->getBody()->getLocStart();
           bool brackWhile=false;
           while(1)//loop to detect whether if is bracketed
           {

                const char *csp=rewriter.getSourceMgr().getCharacterData(wsl);

                if(*csp==';')
                    break;
                if(*csp=='{')
                {
                    brackWhile=true;
                    break;
                }
                 wsl=wsl.getLocWithOffset(1);
           }

           SourceLocation wesl=wstmt->getBody()->getLocEnd();
          // rewriter.InsertText(wstmt->getBody()->getLocEnd(),"^");
           while(1)//finding then end
           {
               const char *csp=rewriter.getSourceMgr().getCharacterData(wesl);
               if(*csp=='\n' )
                   break;
               wesl=wesl.getLocWithOffset(1);
           }
           //string ifsstr,ifestr;
           //rewriter.InsertText(wstmt->getBody()->getLocStart(),"^");
           if(brackWhile)
           {
               errs()<<"inside brack while\n";
               rewriter.ReplaceText(wstmt->getBody()->getLocStart(),"{//Inside while body with condition "+cond+"\n");
               rewriter.ReplaceText(wstmt->getBody()->getLocEnd(),"//Exiting if body with condition "+cond+"\n}");
           }
           else
           {
               errs()<<"inside non brack while\n";
               rewriter.InsertText(wstmt->getBody()->getLocStart(),"{//Inside while body with condition "+cond+"\n");
               rewriter.InsertText(wesl,"//Exiting while body with condition "+cond+"\n}");
           }
           return true;


        }

        bool VisitSwitchStmt(SwitchStmt* st)
        {
            if(!isInMainFile(st->getLocStart()))
               return true ;

           /*SourceRange vdcl= ifst->getConditionVariableDeclStmt()->getSourceRange();
           SourceLocation vbeg(vdcl.getBegin());
           SourceLocation vend(vdcl.getEnd());
           string vstr;

           while(1)
           {
                if(vbeg==vend)
                    break;
                const char *csp=rewriter.getSourceMgr().getCharacterData(vbeg);
                cond+=*csp;
                vbeg=vbeg.getLocWithOffset(1);
           }*/


           SourceLocation sc=st->getCond()->getExprLoc();
           string cond;
           while(1)//exraction condition as string
           {
               const char *csp=rewriter.getSourceMgr().getCharacterData(sc);
               if(*csp==')' )
                   break;
               cond+=*csp;
               sc=sc.getLocWithOffset(1);
           }

           SwitchCase* first=st->getSwitchCaseList();
           while(first)
           {
                SourceLocation s= first->getColonLoc().getLocWithOffset(1);
                rewriter.InsertText(s,"//Inside switch statement condition:"+cond+";\n");
                first=first->getNextSwitchCase();
           }
            return true;
        }

};



class ExampleASTConsumer : public ASTConsumer {
private:
    ExampleVisitor *visitor; // doesn't have to be private

public:
    // override the constructor in order to pass CI
    explicit ExampleASTConsumer(CompilerInstance *CI)
        : visitor(new ExampleVisitor(CI)) // initialize the visitor
    { }

    // override this to call our ExampleVisitor on the entire source file
    virtual void HandleTranslationUnit(ASTContext &Context) {
        /* we can use ASTContext to get the TranslationUnitDecl, which is
             a single Decl that collectively represents the entire source file */
        visitor->TraverseDecl(Context.getTranslationUnitDecl());
    }

/*
    // override this to call our ExampleVisitor on each top-level Decl
    virtual bool HandleTopLevelDecl(DeclGroupRef DG) {
        // a DeclGroupRef may have multiple Decls, so we iterate through each one
        for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; i++) {
            Decl *D = *i;
            visitor->TraverseDecl(D); // recursively visit each AST node in Decl "D"
        }
        return true;
    }
*/
};



class ExampleFrontendAction : public ASTFrontendAction {
public:
    virtual ASTConsumer *CreateASTConsumer(CompilerInstance &CI, StringRef file) {
        return new ExampleASTConsumer(&CI); // pass CI pointer to ASTConsumer
    }
};


// For each source file provided to the tool, a new FrontendAction is created.


int main(int argc, const char **argv) {
    // parse the command-line args passed to your code
    llvm::sys::PrintStackTraceOnErrorSignal();
    CommonOptionsParser op(argc, argv,MyToolCategory);
    // create a new Clang Tool instance (a LibTooling environment)
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    // run the Clang Tool, creating a new FrontendAction (explained below)
    int result = Tool.run(newFrontendActionFactory<ExampleFrontendAction>().get());

    errs() << "\nFound " << numFunctions << " functions.\n\n";
    // print out the rewritten source code ("rewriter" is a global var.)
    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
    
    return result;
}
