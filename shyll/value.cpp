#include "value.h"

Value::Value() : internal()
{
}

Value::Value(double val) : internal(val)
{
}

Value::Value(long val) : internal(val)
{
}

Value::Value(const std::string& val) : internal(val)
{
}

Value::Value(bool val) : internal(val)
{
}

const bool Value::Valid() const
{
	return internal.has_value();
}

const Value Value::operator+(const Value& val) const
{
	if (Get<double>())
	{
		if (val.Get<double>())
		{
			return Value(*Get<double>() + *val.Get<double>());
		}
		else if (val.Get<std::string>())
		{
			return Value(std::to_string(*Get<double>()) + *val.Get<std::string>());
		}
		else if (val.Get<long>())
		{
			return Value(*Get<double>() + *val.Get<long>());
		}
	}
	else if (Get<std::string>())
	{
		if (val.Get<std::string>())
		{
			return Value(*Get<std::string>() + *val.Get<std::string>());
		}
		else if (val.Get<double>())
		{
			return Value(*Get<std::string>() + std::to_string(*val.Get<double>()));
		}
		else if (val.Get<bool>())
		{
			return Value(*Get<std::string>() + (*val.Get<bool>() ? "true" : "false"));
		}
		else if (val.Get<long>())
		{
			return Value(*Get<std::string>() + std::to_string(*val.Get<long>()));
		}
	}
	else if (Get<bool>())
	{
		if (val.Get<std::string>())
		{
			return Value((*Get<bool>() ? "true" : "false") + *val.Get<std::string>());
		}
	}
	else if (Get<long>())
	{
		if (val.Get<double>())
		{
			return Value(*Get<long>() + *val.Get<double>());
		}
		else if (val.Get<std::string>())
		{
			return Value(std::to_string(*Get<long>()) + *val.Get<std::string>());
		}
		else if (val.Get<long>())
		{
			return Value(*Get<long>() + *val.Get<long>());
		}
	}
	return Value();
}

const Value Value::operator-(const Value& val) const
{
	if (Get<double>() && val.Get<double>()) { return Value(*Get<double>() - *val.Get<double>()); }
	if (Get<long>() && val.Get<long>()) { return Value(*Get<long>() - *val.Get<long>()); }
	if (Get<double>() && val.Get<long>()) { return *Get<double>() - *val.Get<long>(); }
	if (Get<long>() && val.Get<double>()) { return *Get<long>() - *val.Get<double>(); }
	return Value();
}

const Value Value::operator*(const Value& val) const
{
	if (Get<double>() && val.Get<double>()) { return Value(*Get<double>() * *val.Get<double>()); }
	if (Get<long>() && val.Get<long>()) { return Value(*Get<long>() * *val.Get<long>()); }
	if (Get<double>() && val.Get<long>()) { return *Get<double>() / *val.Get<long>(); }
	if (Get<long>() && val.Get<double>()) { return *Get<long>() / *val.Get<double>(); }
	return Value();
}

const Value Value::operator/(const Value& val) const
{
	if (Get<double>() && val.Get<double>()) { return Value(*Get<double>() / *val.Get<double>()); }
	if (Get<long>() && val.Get<long>()) { return Value(*Get<long>() / *val.Get<long>()); }
	if (Get<double>() && val.Get<long>()) { return *Get<double>() / *val.Get<long>(); }
	if (Get<long>() && val.Get<double>()) { return *Get<long>() / *val.Get<double>(); }
	return Value();
}

const Value Value::operator<(const Value& val) const
{
	if (Get<double>() && val.Get<double>()) { return *Get<double>() < *val.Get<double>(); }
	if (Get<long>() && val.Get<long>()) { return *Get<long>() < *val.Get<long>(); }
	if (Get<double>() && val.Get<long>()) { return *Get<double>() < *val.Get<long>(); }
	if (Get<long>() && val.Get<double>()) { return *Get<long>() < *val.Get<double>(); }
	return Value();
}

const Value Value::operator<=(const Value& val) const
{
	if (Get<double>() && val.Get<double>()) { return *Get<double>() <= *val.Get<double>(); }
	if (Get<long>() && val.Get<long>()) { return *Get<long>() <= *val.Get<long>(); }
	if (Get<double>() && val.Get<long>()) { return *Get<double>() <= *val.Get<long>(); }
	if (Get<long>() && val.Get<double>()) { return *Get<long>() <= *val.Get<double>(); }
	return Value();
}

const Value Value::operator>(const Value& val) const
{
	if (Get<double>() && val.Get<double>()) { return *Get<double>() > *val.Get<double>(); }
	if (Get<long>() && val.Get<long>()) { return *Get<long>() > *val.Get<long>(); }
	if (Get<double>() && val.Get<long>()) { return *Get<double>() > *val.Get<long>(); }
	if (Get<long>() && val.Get<double>()) { return *Get<long>() > *val.Get<double>(); }
	return Value();
}

const Value Value::operator>=(const Value& val) const
{
	if (Get<double>() && val.Get<double>()) { return *Get<double>() >= *val.Get<double>(); }
	if (Get<long>() && val.Get<long>()) { return *Get<long>() >= *val.Get<long>(); }
	if (Get<double>() && val.Get<long>()) { return *Get<double>() >= *val.Get<long>(); }
	if (Get<long>() && val.Get<double>()) { return *Get<long>() >= *val.Get<double>(); }
	return Value();
}

const Value Value::operator==(const Value& val) const
{
	if (Get<double>() && val.Get<double>()) { return *Get<double>() == *val.Get<double>(); }
	if (Get<std::string>() && val.Get<std::string>()) { return *Get<std::string>() == *val.Get<std::string>(); }
	if (Get<bool>() && val.Get<bool>()) { return *Get<bool>() == *val.Get<bool>(); }
	if (Get<long>() && val.Get<long>()) { return *Get<long>() == *val.Get<long>(); }
	return Value();
}

const Value Value::operator!=(const Value& val) const
{
	if (Get<double>() && val.Get<double>()) { return *Get<double>() != *val.Get<double>(); }
	if (Get<std::string>() && val.Get<std::string>()) { return *Get<std::string>() != *val.Get<std::string>(); }
	if (Get<bool>() && val.Get<bool>()) { return *Get<bool>() != *val.Get<bool>(); }
	if (Get<long>() && val.Get<long>()) { return *Get<long>() != *val.Get<long>(); }
	return Value();
}

const Value Value::operator&&(const Value& val) const
{
	if (Get<bool>() && val.Get<bool>()) { return *Get<bool>() && *val.Get<bool>(); }
	return Value();
}

const Value Value::operator||(const Value& val) const
{
	if (Get<bool>() && val.Get<bool>()) { return *Get<bool>() || *val.Get<bool>(); }
	return Value();
}

const Value Value::operator!() const
{
	if (Get<bool>()) { return !*Get<bool>(); }
	return Value();
}

std::ostream& operator<<(std::ostream& stream, const Value& value)
{
	if (value.internal.has_value())
	{
		switch (value.internal.value().index())
		{
		case 0:
			return stream << std::get<double>(value.internal.value());

		case 1:
			return stream << std::get<std::string>(value.internal.value());

		case 2:
			return stream << (std::get<bool>(value.internal.value()) ? "true" : "false");

		case 3:
			return stream << std::get<long>(value.internal.value());
		}
	}
	return stream;
}
