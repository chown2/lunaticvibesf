#pragma once
#include "difficultytable.h"

class DifficultyTableBMS: public DifficultyTable
{
public:
	DifficultyTableBMS() = default;
	~DifficultyTableBMS() override = default;

protected:
	std::string symbol;
	std::string data_url;

public:
	void updateFromUrl(std::function<void(UpdateResult)> finishedCallback) override;

	bool loadFromFile() override;

	Path getFolderPath() const;

	std::string getSymbol() const;

	void parseHeader(const std::string& content);
	void parseBody(const std::string& content);
};