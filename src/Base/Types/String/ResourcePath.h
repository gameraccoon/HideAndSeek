#pragma once

#include <filesystem>

#include <nlohmann/json_fwd.hpp>

/**
 * Path to a resource relative to the resource directory
 *
 * Use this path to store resource paths in components, world data, saves (etc.)
 */
class RelativeResourcePath
{
public:
	RelativeResourcePath() = default;

	template<typename RelativePath, std::enable_if_t<!std::is_same_v<std::decay_t<RelativePath>, RelativeResourcePath>, int> = 0>
	explicit RelativeResourcePath(RelativePath&& relativePath)
		: mRelativePath(std::forward<RelativePath>(relativePath))
	{
		if (mRelativePath.is_absolute())
		{
			ReportError("Absolute path '%s' disguised as relative", mRelativePath.string().c_str());
			mRelativePath = "";
		}
	}

	~RelativeResourcePath() = default;
	RelativeResourcePath(const RelativeResourcePath&) = default;
	RelativeResourcePath& operator=(const RelativeResourcePath&) = default;
	RelativeResourcePath(RelativeResourcePath&&) noexcept = default;
	RelativeResourcePath& operator=(RelativeResourcePath&&) noexcept = default;

	bool operator==(const RelativeResourcePath& other) const = default;
	bool operator!=(const RelativeResourcePath& other) const = default;

	const std::filesystem::path& getRelativePath() const& noexcept { return mRelativePath; }
	std::filesystem::path&& getRelativePath() && noexcept { return std::move(mRelativePath); }
	std::string getRelativePathStr() const { return mRelativePath; }

private:
	std::filesystem::path mRelativePath;
};

/**
 * Absolute path to a resource calculated from relative path and resource directory path
 *
 * Don't store this path offline, only use for runtime
 */
class AbsoluteResourcePath
{
public:
	AbsoluteResourcePath() = default;

	template<typename ResourceDirectoryPath, typename RelativePath, std::enable_if_t<std::is_same_v<std::decay_t<RelativePath>, RelativeResourcePath>, int> = 0>
	explicit AbsoluteResourcePath(ResourceDirectoryPath&& resourceDirectoryPath, RelativePath&& relativePath)
		: mAbsolutePath(std::forward<ResourceDirectoryPath>(resourceDirectoryPath) / std::forward<RelativePath>(relativePath).getRelativePath())
	{
	}

	~AbsoluteResourcePath() = default;
	AbsoluteResourcePath(const AbsoluteResourcePath&) = default;
	AbsoluteResourcePath& operator=(const AbsoluteResourcePath&) = default;
	AbsoluteResourcePath(AbsoluteResourcePath&&) noexcept = default;
	AbsoluteResourcePath& operator=(AbsoluteResourcePath&&) noexcept = default;

	bool operator==(const AbsoluteResourcePath& other) const = default;
	bool operator!=(const AbsoluteResourcePath& other) const = default;

	std::string getAbsolutePathStr() const { return mAbsolutePath.string(); }
	const std::filesystem::path& getAbsolutePath() const { return mAbsolutePath; }

private:
	std::filesystem::path mAbsolutePath;
};

namespace std
{
	template<> struct hash<RelativeResourcePath>
	{
		size_t operator()(RelativeResourcePath const& path) const noexcept
		{
			return hash<std::filesystem::path>{}(path.getRelativePath());
		}
	};

	template<> struct hash<AbsoluteResourcePath>
	{
		size_t operator()(AbsoluteResourcePath const& path) const noexcept
		{
			return hash<std::filesystem::path>{}(path.getAbsolutePath());
		}
	};
}

void to_json(nlohmann::json& outJson, const RelativeResourcePath& path);
void from_json(const nlohmann::json& json, RelativeResourcePath& path);
