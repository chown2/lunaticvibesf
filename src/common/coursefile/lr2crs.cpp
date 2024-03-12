#include "lr2crs.h"

#include <fstream>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "common/encoding.h"
#include "common/hash.h"
#include "common/log.h"
#include "common/utils.h"
#include "common/sysutil.h"

CourseLr2crs::CourseLr2crs(const Path& filePath)
{
	// .lr2crs is a standard xml file. A file may contain muiltiple courses.

	// load file
	std::ifstream ifs(filePath);
	if (ifs.fail())
	{
		LOG_WARNING << "[lr2crs] " << filePath << " File ERROR";
		return;
	}
	std::stringstream ss;
	ss << ifs.rdbuf();
	ss.sync();
	ifs.close();

	addTime = getFileLastWriteTime(filePath);

	// convert codepage
	auto encoding = getFileEncoding(ss);
	std::stringstream ssUTF8;
	std::string lineBuf;
	while (!ss.eof())
	{
		std::getline(ss, lineBuf);

		// convert codepage
		lineBuf = to_utf8(lineBuf, encoding);
		lunaticvibes::trim_in_place(lineBuf);

		ssUTF8 << lineBuf;
	}
	ssUTF8.sync();
	ss.clear();

	// parse as XML
	try
	{
		namespace pt = boost::property_tree;
		pt::ptree tree;
		pt::read_xml(ssUTF8, tree);
		for (auto& course : tree.get_child("courselist"))
		{
			courses.emplace_back();
			Course& c = courses.back();
			for (auto& [name, value] : course.second)
			{
				if (lunaticvibes::iequals(name, "title"))
					c.title = value.data();
				else if (lunaticvibes::iequals(name, "line"))
					c.line = toInt(value.data());
				else if (lunaticvibes::iequals(name, "type"))
					c.type = decltype(c.type)(toInt(value.data()));
				else if (lunaticvibes::iequals(name, "hash"))
				{
					// first 32 characters are course metadata, of which we didn't make use; the 10th char is course type, the last 4 are assumed to be uploader ID
					// after that, each 32 chars indicates a chart MD5
					const std::string& hash = value.data();
					c.hashTop = hash.substr(0, 32);
					for (size_t count = 1; count < hash.size() / 32; ++count)
					{
						c.chartHash.emplace_back(hash.substr(count * 32, 32));
					}
				}
			}
		}
	}
	catch (...)
	{
		courses.clear();
	}
}

HashMD5 CourseLr2crs::Course::getCourseHash() const
{
	std::stringstream ss;
	ss << (int)type;
	ss << chartHash.size();
	for (auto& c : chartHash) ss << c.hexdigest();
	return md5(ss.str());
}