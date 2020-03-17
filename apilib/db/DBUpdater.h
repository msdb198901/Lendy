#ifndef DB_UPDATER_H
#define DB_UPDATER_H

#include "Define.h"
#include "DBEnvHeader.h"
#include <set>
#include <unordered_map>
#include <string>
#include <filesystem>

namespace DB
{
	typedef std::tr2::sys::path Path;
	using namespace std::tr2::sys;

	///////////////////提取器////////////////
	enum UpdateMode
	{
		MODE_APPLY,
		MODE_REHASH
	};

	enum State
	{
		RELEASED,
		ARCHIVED
	};

	struct UpdateResult;
	struct AppliedFileEntry;
	struct DirectoryEntry;
	
	class UpdateFetcher
	{
	public:
		UpdateFetcher(Path const& updateDirectory,
			std::function<void(std::string const&)> const& apply,
			std::function<void(Path const& path)> const& applyFile,
			std::function<QueryResult(std::string const&)> const& retrieve);

		~UpdateFetcher();

		UpdateResult Update(bool const redundancyChecks, bool const allowRehash,
			bool const archivedRedundancy, int32 const cleanDeadReferencesMaxCount) const;

	private:
		
		typedef std::pair<Path, State> LocaleFileEntry;
		struct PathCompare
		{
			bool operator()(LocaleFileEntry const& left, LocaleFileEntry const& right) const;
		};

		typedef std::set<LocaleFileEntry, PathCompare> LocaleFileStorage;
		typedef std::unordered_map<std::string, std::string> HashToFileNameStorage;
		typedef std::unordered_map<std::string, AppliedFileEntry> AppliedFileStorage;
		typedef std::vector<DirectoryEntry> DirectoryStorage;

		LocaleFileStorage GetFileList() const;
		void FillFileListRecursively(Path const& path, LocaleFileStorage& storage,
			State const state, uint32 const depth) const;

		DirectoryStorage ReceiveIncludedDirectories() const;
		AppliedFileStorage ReceiveAppliedFiles() const;

		std::string ReadSQLUpdate(Path const& file) const;

		uint32 Apply(Path const& path) const;

		void UpdateEntry(AppliedFileEntry const& entry, uint32 const speed = 0) const;
		void RenameEntry(std::string const& from, std::string const& to) const;
		void CleanUp(AppliedFileStorage const& storage) const;

		void UpdateState(std::string const& name, State const state) const;

		std::unique_ptr<Path> const _sourceDirectory;
		std::function<void(std::string const&)> const _apply;
		std::function<void(Path const& path)> const _applyFile;
		std::function<QueryResult(std::string const&)> const _retrieve;
	};
	////////////////////////////


	///////更新类//////////
	template <class T>
	class DBWorkerPool;

	enum BaseLocation
	{
		LOCATION_REPOSITORY,
		LOCATION_DOWNLOAD
	};

	template <class T>
	class LENDY_COMMON_API DBUpdater
	{
	public:
		static bool Create(DBWorkerPool<T>& pool);

		static bool Populate(DBWorkerPool<T>& pool);

		static bool Update(DBWorkerPool<T>& pool);

		static inline std::string GetTableName();

		static inline std::string GetSourceDirectory();

		static inline  std::string GetBaseFile();

		static BaseLocation GetBaseLocationType();

		static QueryResult Retrieve(DBWorkerPool<T>& pool, std::string const& query);

		static void Apply(DBWorkerPool<T>& pool, std::string const& query);

	private:
		static void ApplyFile(DBWorkerPool<T>& pool, Path const& path);
		static void ApplyFile(DBWorkerPool<T>& pool, std::string const& host, std::string const& user,
			std::string const& password, std::string const& portsocket, std::string const& database, Path const& path);
		
		//mysql路径
		static std::string& CorrectedPath();

	private:
		//检查mysql应用程序
		static bool CheckExecutable();
	};
}

#endif