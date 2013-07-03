#ifndef _VECTOR2_H_
#define _VECTOR2_H_


template<typename Derived>
inline bool is_valid(const Derived & x)
{
	return (x == x) && ((x - x) == (x - x));
}

template <typename Derived>
inline Derived clamp(Derived left, Derived min, Derived max)
{
	return (left < min ? min : (left > max ? max : left));
}

//------------------------------------------------------------------------------------------------------------------

//Taken from the SFML Vector2 class:
//https://github.com/LaurentGomila/SFML/blob/master/include/SFML/System/Vector2.hpp
//https://github.com/LaurentGomila/SFML/blob/master/include/SFML/System/Vector2.inl

template <typename T>
class Vector2
{
public:
	Vector2() : x(0), y(0)
	{
	}

	Vector2(T XY) : x(XY), y(XY)
	{
	}

	Vector2(T X, T Y) : x(X), y(Y)
	{
	}

	//convert between vector types
	template <typename U>
    explicit Vector2(const Vector2<U>& vector) : x(static_cast<T>(vector.x)), y(static_cast<T>(vector.y))
	{
	}

	//convert between vector types
	template <typename U>
	operator Vector2<U> ()
	{
		return Vector2<U>(*this);
	}
	
	T x;
	T y;
};

template <typename T>
Vector2<T> operator -(const Vector2<T>& right)
{
	return Vector2<T>(-right.x, -right.y);
}

template <typename T>
Vector2<T>& operator +=(Vector2<T>& left, const Vector2<T>& right)
{
	left.x += right.x;
	left.y += right.y;

	return left;
}

template <typename T>
Vector2<T>& operator -=(Vector2<T>& left, const Vector2<T>& right)
{
	left.x -= right.x;
	left.y -= right.y;

	return left;
}

template <typename T>
Vector2<T> operator +(const Vector2<T>& left, const Vector2<T>& right)
{
	return Vector2<T>(left.x + right.x, left.y + right.y);
}

template <typename T>
Vector2<T> operator +(const Vector2<T>& left, T right)
{
	return Vector2<T>(left.x + right, left.y + right);
}

template <typename T>
Vector2<T> operator -(const Vector2<T>& left, const Vector2<T>& right)
{
	return Vector2<T>(left.x - right.x, left.y - right.y);
}

template <typename T>
Vector2<T> operator -(const Vector2<T>& left, T right)
{
	return Vector2<T>(left.x - right, left.y - right);
}

template <typename T>
Vector2<T> operator *(const Vector2<T>& left, T right)
{
	return Vector2<T>(left.x * right, left.y * right);
}

template <typename T>
Vector2<T>& operator *=(Vector2<T>& left, T right)
{
	left.x *= right;
	left.y *= right;

	return left;
}

template <typename T>
Vector2<T> cWiseProd(const Vector2<T>& left, const Vector2<T>& right)
{
	return Vector2<T>(left.x * right.x, left.y * right.y);
}

template <typename T>
Vector2<T> operator /(const Vector2<T>& left, T right)
{
	return Vector2<T>(left.x / right, left.y / right);
}

template <typename T>
Vector2<T>& operator /=(Vector2<T>& left, T right)
{
	left.x /= right;
	left.y /= right;

	return left;
}

template <typename T>
Vector2<T> cWiseQuot(const Vector2<T>& left, const Vector2<T>& right)
{
	return Vector2<T>(left.x / right.x, left.y / right.y);
}

template <typename T>
bool operator ==(const Vector2<T>& left, const Vector2<T>& right)
{
	return (left.x == right.x && left.y == right.y);
}

template <typename T>
bool operator !=(const Vector2<T>& left, const Vector2<T>& right)
{
	return (left.x != right.x) || (left.y != right.y);
}

template <typename T>
bool operator >(const Vector2<T>& left, const Vector2<T>& right)
{
	return (left.x > right.x && left.y > right.y);
}

template <typename T>
bool operator >=(const Vector2<T>& left, const Vector2<T>& right)
{
	return (left.x >= right.x && left.y >= right.y);
}

template <typename T>
bool operator <(const Vector2<T>& left, const Vector2<T>& right)
{
	return (left.x < right.x && left.y < right.y);
}

template <typename T>
bool operator <=(const Vector2<T>& left, const Vector2<T>& right)
{
	return (left.x <= right.x && left.y <= right.y);
}

template <typename T>
Vector2<T> abs(const Vector2<T>& right)
{
	return Vector2<T>(abs(right.x), abs(right.y));
}

template <typename T>
Vector2<T> clamp(const Vector2<T> left, const Vector2<T> min, const Vector2<T> max)
{
	return Vector2<T>(left.x < min.x ? min.x : (left.x > max.x ? max.x : left.x), left.y < min.y ? min.y : (left.y > max.y ? max.y : left.y));
}

typedef Vector2<int> Vector2i;
typedef Vector2<unsigned int> Vector2u;
typedef Vector2<float> Vector2f;

//------------------------------------------------------------------------------------------------------------------

template <typename U>
class Rect
{
public:
	Vector2<U> pos;
	Vector2<U> size;

	Rect() {};
	Rect(typename U x, typename U y, typename U w, typename U h) : pos(x, y), size(w, h) {};
	Rect(Vector2<U> p, Vector2<U> s) : pos(p), size(s) {};
};

typedef Rect<int> Recti;
typedef Rect<unsigned int> Rectu;
typedef Rect<float> Rectf;

#endif
