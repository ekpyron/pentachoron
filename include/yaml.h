/*  
 * This file is part of DRE.
 *
 * DRE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DRE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DRE.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef YAML_H
#define YAML_H

#ifndef COMMON_H
#error "Don't include yaml.h directly. Use common.h instead."
#endif

#include <yaml-cpp/yaml.h>

namespace YAML {
template<typename glmT>
struct convert<glm::detail::tvec3<glmT> > {
	 static Node encode (const glm::detail::tvec3<glmT> &v) {
		 Node node;
		 node.push_back (v.x);
		 node.push_back (v.y);
		 node.push_back (v.z);
		 return node;
	 }
	 static bool decode (const Node &node, glm::detail::tvec3<glmT> &v) {
		 if (!node.IsSequence ())
				return false;
		 if (!node.size () == 3)
				return false;
		 try {
			 v.x = node[0].as<glmT> ();
			 v.y = node[1].as<glmT> ();
			 v.z = node[2].as<glmT> ();
			 return true;
		 } catch (...) {
			 return false;
		 }
	 }
};

template<typename glmT>
struct convert<glm::detail::tvec4<glmT> > {
	 static Node encode (const glm::detail::tvec4<glmT> &v) {
		 Node node;
		 node.push_back (v.x);
		 node.push_back (v.y);
		 node.push_back (v.z);
		 node.push_back (v.w);
		 return node;
	 }
	 static bool decode (const Node &node, glm::detail::tvec4<glmT> &v) {
		 if (!node.IsSequence ())
				return false;
		 if (!node.size () == 4)
				return false;
		 try {
			 v.x = node[0].as<glmT> ();
			 v.y = node[1].as<glmT> ();
			 v.z = node[2].as<glmT> ();
			 v.w = node[3].as<glmT> ();
			 return true;
		 } catch (...) {
			 return false;
		 }
	 }
};
}

#endif /* YAML_H */
