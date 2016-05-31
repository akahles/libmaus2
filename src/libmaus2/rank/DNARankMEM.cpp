/*
    libmaus2
    Copyright (C) 2016 German Tischler

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <libmaus2/rank/DNARankMEM.hpp>

std::ostream & libmaus2::rank::operator<<(std::ostream & out, libmaus2::rank::DNARankMEM const & D)
{
	return out << "DNARankMEM(intv=" << D.intv << ",left=" << D.left << ",right=" << D.right << ")";
}