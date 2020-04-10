#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <variant>

class Value
{
public:
	Value();
	Value(double val);
	Value(long val);
	Value(const std::string& val);
	Value(bool val);

	friend std::ostream& operator<<(std::ostream& stream, const Value& value);

	template <typename T>
	const T* Get() const
	{
		if (internal.has_value())
		{
			return std::get_if<T>(&internal.value());
		}
		else
		{
			return nullptr;
		}
	}

	const bool Valid() const;

	const Value operator+(const Value& val) const;
	const Value operator-(const Value& val) const;
	const Value operator*(const Value& val) const;
	const Value operator/(const Value& val) const;
	const Value operator<(const Value& val) const;
	const Value operator<=(const Value& val) const;
	const Value operator>(const Value& val) const;
	const Value operator>=(const Value& val) const;
	const Value operator==(const Value& val) const;
	const Value operator!=(const Value& val) const;
	const Value operator&&(const Value& val) const;
	const Value operator||(const Value& val) const;

	const Value operator-() const;
	const Value operator!() const;

private:
	typedef std::variant<double, std::string, bool, long> ValueVariant;

	std::optional<ValueVariant> internal;
};