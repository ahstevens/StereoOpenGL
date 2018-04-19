#include "DistortionUtils.h"

#include <gtx/intersect.hpp>
#include <gtx/vector_angle.hpp>

namespace distutil
{
	std::vector<glm::vec3> transformMonoscopicPoints(glm::vec3 centerOfProj, glm::vec3 viewPos, glm::vec3 screenCtr, glm::vec3 screenNorm, std::vector<glm::vec3> obj)
	{
		auto intPts = getScreenIntersections(centerOfProj, screenCtr, screenNorm, obj);

		std::vector<glm::vec3> ret;

		for (int i = 0; i < obj.size(); ++i)
		{
			float ratio = glm::length(obj[i] - intPts[i]) / glm::length(intPts[i] - centerOfProj);
			glm::vec3 newPosToInt = intPts[i] - viewPos;
			float transformedOffset = ratio * glm::length(newPosToInt);

			ret.push_back(intPts[i] + glm::normalize(newPosToInt) * transformedOffset);
		}

		return ret;
	}

	std::vector<glm::vec3> transformStereoscopicPoints(glm::vec3 centerOfProjL, glm::vec3 centerOfProjR, glm::vec3 viewPosL, glm::vec3 viewPosR, glm::vec3 screenCtr, glm::vec3 screenNorm, std::vector<glm::vec3> obj)
	{
		std::vector<glm::vec3> iL(getScreenIntersections(centerOfProjL, screenCtr, screenNorm, obj));
		std::vector<glm::vec3> iR(getScreenIntersections(centerOfProjR, screenCtr, screenNorm, obj));

		std::vector<glm::vec3> ret;
		for (int i = 0; i < obj.size(); ++i)
		{
			glm::vec3 pa, pb;
			double mua, mub;
			LineLineIntersect(viewPosL, iL[i], viewPosR, iR[i], &pa, &pb, &mua, &mub);
			ret.push_back((pa + pb) / 2.f);
		}

		return ret;
	}

	std::vector<glm::vec3> getScreenIntersections(glm::vec3 centerOfProjection, glm::vec3 screenCenter, glm::vec3 screenNormal, std::vector<glm::vec3> pts)
	{
		std::vector<glm::vec3> ret;

		for (auto pt : pts)
		{
			float ptDist;
			glm::intersectRayPlane(centerOfProjection, pt - centerOfProjection, screenCenter, screenNormal, ptDist);
			ret.push_back(centerOfProjection + (pt - centerOfProjection) * ptDist);
		}
		return ret;
	}

	/*
	from http://paulbourke.net/geometry/pointlineplane/lineline.c

	Calculate the line segment PaPb that is the shortest route between
	two lines P1P2 and P3P4. Calculate also the values of mua and mub where
	Pa = P1 + mua (P2 - P1)
	Pb = P3 + mub (P4 - P3)
	Return false if no solution exists.
	*/
	bool LineLineIntersect(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 *pa, glm::vec3 *pb, double *mua, double *mub)
	{
		glm::vec3 p13, p43, p21;
		double d1343, d4321, d1321, d4343, d2121;
		double numer, denom;

		p13.x = p1.x - p3.x;
		p13.y = p1.y - p3.y;
		p13.z = p1.z - p3.z;
		p43.x = p4.x - p3.x;
		p43.y = p4.y - p3.y;
		p43.z = p4.z - p3.z;
		if (glm::abs(p43.x) < glm::epsilon<float>() && glm::abs(p43.y) < glm::epsilon<float>() && glm::abs(p43.z) < glm::epsilon<float>())
			return false;
		p21.x = p2.x - p1.x;
		p21.y = p2.y - p1.y;
		p21.z = p2.z - p1.z;
		if (glm::abs(p21.x) < glm::epsilon<float>() && glm::abs(p21.y) < glm::epsilon<float>() && glm::abs(p21.z) < glm::epsilon<float>())
			return false;

		d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
		d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
		d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
		d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
		d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

		denom = d2121 * d4343 - d4321 * d4321;
		if (glm::abs(denom) < glm::epsilon<float>())
			return false;
		numer = d1343 * d4321 - d1321 * d4343;

		*mua = numer / denom;
		*mub = (d1343 + d4321 * (*mua)) / d4343;

		pa->x = p1.x + *mua * p21.x;
		pa->y = p1.y + *mua * p21.y;
		pa->z = p1.z + *mua * p21.z;
		pb->x = p3.x + *mub * p43.x;
		pb->y = p3.y + *mub * p43.y;
		pb->z = p3.z + *mub * p43.z;

		return true;
	}
}
