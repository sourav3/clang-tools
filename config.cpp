# include <iostream>
# include <fstream>
# include <map>
# include <sstream>
# include <sys/stat.h>
# include "llvm/ADT/StringRef.h"

namespace logConfig
{
	using namespace std;
	string IfLogIn ;
	string IfLogOut ;
	string ElseLogIn;
	string ElseLogOut;
	string ForLog ;
	string WhileLog ;
	string DoWhileLog;
	string SwitchLog ;

	map<string,string>logst;

	void fill_strings()
	{
        IfLogIn=logst["IfLogIn"];
        IfLogOut=logst["IfLogOut"];
        ElseLogIn=logst["ElseLogIn"];
        ElseLogOut=logst["ElseLogOut"];
        ForLog=logst["ForLog"];
        WhileLog=logst["WhileLog"];
        DoWhileLog=logst["DoWhileLog"];
        SwitchLog=logst["SwitchLog"];
        

	}

	inline bool exists_test3 (const std::string& name)
	{
	  struct stat buffer;
	  return (stat (name.c_str(), &buffer) == 0);
	}

	void LoadLog(string filename="app.config")
	{
		//using namespace std;
		using namespace llvm;
		fstream file(filename.c_str());
		if(!file.good())
            return;
		string log;
		while(getline(file,log))
		{

			StringRef sref(log);
			auto lp= sref.split(':');
			lp.first=lp.first.trim();
			logst[lp.first.str()]=lp.second.str();
			//cout<<lp.first.str()<<lp.second.str()<<endl;
		}
		fill_strings();
	}

	void display()
	{
		cout<<"\n\n Testing configuration Load \n";
		cout<<"IfLogIn :"<<logst["IfLogIn"]<<endl;
		cout<<"IfLogOut :"<<logst["IflogOut"]<<endl;
		cout<<"ElseLogIn :"<<logst["ElseLogIn"]<<endl;
		cout<<"ElseLogOut:"<<logst["ElseLogOut"]<<endl;
		cout<<"For :"<<logst["ForLog"]<<endl;
		cout<<"While :"<<logst["WhileLog"]<<endl;
		cout<<"Switch :"<<logst["SwitchLog"]<<endl;

	}


}

