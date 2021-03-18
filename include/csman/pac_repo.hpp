#pragma once
/*
 * CovScript Manager: Package Repository
 * Licensed under Apache 2.0
 * Copyright (C) 2020-2021 Chengdu Covariant Technologies Co., LTD.
 * Website: https://covariant.cn/
 * Github:  https://github.com/chengdu-zhirui/
 */
#include <csman/global.hpp>
#include <iostream>
#include <fstream>

namespace csman {
	class pac_repo final {
	private:
		context *cxt = nullptr;
	public:/*公开类*/
		class pac_data {
		public:
			std::string available = "";
			set_t<std::string> ver;
			pac_data():ver() {} // 怎么保证使用map可以正确初始化
			pac_data(const std::vector<std::string> &args)
			{
				available = args[1];
				for(int i = 1; i<args.size() ; i++) {
					ver.insert(args[i]);
				}
			}
		};

		map_t<std::string, pac_data> local_pac;
    public:/*公开接口*/
		void read_pac_list(const std::string &path)
		{
			std::ifstream ifs(path + delimiter + "pac_list");
			if(!ifs.is_open())
				throw std::runtime_error("opening \"pac_list\" failed.");
			std::vector<std::string> args;
			while (str::readline(ifs, args))
				local_pac[args[0]] = pac_data(args);
			ifs.close();
		}

		void write_pac_list(const std::string &path)
		{
			std::ofstream ofs(path + delimiter + "pac_list");
			for (auto &x : local_pac) {
				ofs << x.first << " ";
				ofs << x.second.available << " ";
				for (auto &y : x.second.ver)
					if (y != x.second.available)
						ofs << y << " ";
				ofs << std::endl;
			}
			ofs.close();
		}

		pac_repo() = default;

		explicit pac_repo(context *cxt) : cxt(cxt)
		{
			if(cxt->vars.count("pac_repo_path") != 0) {
				try {
				    initialize_local_pac();
				}
				catch (const std::exception &e) {
					throw std::runtime_error("initializing pac_list failed: "+std::string(e.what()));
				}
			}
			else
				throw std::runtime_error("csman has a bug which can not get variable named \"pac_repo_path\" in context.");
			return;
		}

		~pac_repo()
		{
			if (cxt != nullptr) {
				if(cxt->vars.count("pac_repo_path") != 0)
					write_pac_list(cxt->vars["pac_repo_path"]);
				else
					std::cout<<"Warning: your file for recording pac_repo updating failed, it may cause extremely problems while next last_update_time. Please try to repair it by using \"csman repair\""<<std::endl;
			}
		}

		void initialize_local_pac() {
            std::string path = cxt->vars["pac_repo_path"];
            if(!sys::exist(path)){
                sys::dir::create(path);
                sys::file::create(path+delimiter+"pac_list");
            }
            else{
                for(auto &pac : sys::dir::scan(path)){
                    for(auto &ver : sys::dir::scan(path+delimiter+pac))
                        local_pac[pac].ver.insert(ver);
                }
            }
		}
		/*还差安装runtime特殊处理*/
		void update_install(const std::string &name, const std::string &ver, bool is_available)
		{
			if (local_pac.count(name) == 0)
				local_pac[name] = pac_data();
			if (is_available)
				local_pac[name].available = ver;
			local_pac[name].ver.insert(ver);

			return;
		}
		/*还差安装runtime特殊处理*/
		void update_uninstall(const std::string &name, const std::string &ver)
		{
			if (local_pac.count(name) == 0)
				throw std::invalid_argument("package \"" + name + "\" is not existed.");

			auto it = local_pac[name].ver.find(ver);

			if (it == local_pac[name].ver.end())
				throw std::invalid_argument("package \"" + name + "\" has no such version \"" + ver + "\".");

			if (local_pac[name].available == ver)
				local_pac[name].available.clear();
			local_pac[name].ver.erase(ver);
			return;
		}

		void update_checkout()
		{

		}

		inline set_t<std::string> query_contains_ver(const std::string &name)
		{
			set_t<std::string> res;
			if (local_pac.count(name) != 0) {
				for (auto ver : local_pac[name].ver)
					res.insert(ver);
			}
			return res;
		}

		inline std::string query_using_ver(const std::string &name)
		{
			return local_pac.count(name) == 0 ? "" : /*likely*/ local_pac[name].available;
		}

		inline std::string get_current_runtime_ver()
		{
			return cxt->runtime_ver;
		}
	};
}