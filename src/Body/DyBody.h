/**
   \file
   \author Shin'ichiro Nakaoka
*/

#ifndef CNOID_BODY_DYBODY_H
#define CNOID_BODY_DYBODY_H

#include "Body.h"
#include "Link.h"
#include "exportdecl.h"

namespace cnoid {

/**
   A Link class used for forward dynamics based on the articulated body method (ABM)
*/
class CNOID_EXPORT DyLink : public Link
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    DyLink();
    DyLink(const Link& link);
    virtual Link* clone() const;

    virtual void initializeState() override;

    DyLink* parent() const { return static_cast<DyLink*>(Link::parent()); }
    DyLink* sibling() const { return static_cast<DyLink*>(Link::sibling()); }
    DyLink* child() const { return static_cast<DyLink*>(Link::child()); }

    const Vector3& vo() const { return vo_; }
    Vector3& vo() { return vo_; }
    const Vector3& dvo() const { return dvo_; }
    Vector3& dvo() { return dvo_; }
    const Vector3& sw() const { return sw_; }
    Vector3& sw() { return sw_; }
    const Vector3& sv() const { return sv_; }
    Vector3& sv() { return sv_; }
    const Vector3& cv() const { return cv_; }
    Vector3& cv() { return cv_; }
    const Vector3& cw() const { return cw_; }
    Vector3& cw() { return cw_; }
    const Matrix3& Iww() const { return Iww_; }
    Matrix3& Iww() { return Iww_; }
    const Matrix3& Iwv() const { return Iwv_; }
    Matrix3& Iwv() { return Iwv_; }
    const Matrix3& Ivv() const { return Ivv_; }
    Matrix3& Ivv() { return Ivv_; }
    const Vector3& pf() const { return pf_; }
    Vector3& pf() { return pf_; }
    const Vector3& ptau() const { return ptau_; }
    Vector3& ptau() { return ptau_; }
    const Vector3& hhv() const { return hhv_; }
    Vector3& hhv() { return hhv_; }
    const Vector3& hhw() const { return hhw_; }
    Vector3& hhw() { return hhw_; }
    double uu() const { return uu_; }
    double& uu() { return uu_; }
    double dd() const { return dd_; }
    double& dd() { return dd_; }

    /**
       This is used for checking or visualizing the actual constraint forces.
       This is updated if ConstraintForceSolver::enableConstraintForceOutput(true) is called.
    */
    struct ConstraintForce {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        ConstraintForce(const Vector3& point, const Vector3& force) : point(point), force(force) { }
        Vector3 point;
        Vector3 force;
    };
    typedef std::vector<ConstraintForce> ConstraintForceArray;
    
    ConstraintForceArray& constraintForces() { return constraintForces_; }
    const ConstraintForceArray& constraintForces() const { return constraintForces_; }

    virtual void prependChild(Link* link);
    virtual void appendChild(Link* link);
        
private:
    Vector3 vo_;  ///< translation elements of spacial velocity
    Vector3 dvo_; ///< derivative of vo
    
    /**
       A unit vector of angular velocity (the world coordinate) generated by the joint 
       The value is parent->R * a when the joint is the rotational type.
    */
    Vector3 sw_;
    
    /**
       A unit vector of spatial velocity (the world coordinate) generated by the joint.
       The value is parent->R * d when the joint is the translation type.
    */
    Vector3 sv_;
	
    Vector3 cv_;   ///< dsv * dq (cross velocity term)
    Vector3 cw_;   ///< dsw * dq (cross velocity term)

    Matrix3 Iww_;  ///< bottm right block of the articulated inertia
    Matrix3 Iwv_;  ///< bottom left block (transpose of top right block) of the articulated inertia
    Matrix3 Ivv_;  ///< top left block of the articulated inertia
    Vector3 pf_;   ///< bias force (linear element)
    Vector3 ptau_; ///< bias force (torque element)
    Vector3 hhv_;  ///< top block of Ia * s
    Vector3 hhw_;  ///< bottom bock of Ia * s 
    double uu_;
    double dd_;    ///< Ia * s*s^T

    std::vector<ConstraintForce> constraintForces_;
};


/**
   A Body class used for forward dynamics based on the articulated body method (ABM)
*/
class CNOID_EXPORT DyBody : public Body
{
public:
    DyBody();
    DyBody(const Body& org);

    virtual Body* clone() const;
    virtual Link* createLink(const Link* org = 0) const;

    DyLink* joint(int id) const {
        return static_cast<DyLink*>(Body::joint(id));
    }

    DyLink* link(int index) const {
        return static_cast<DyLink*>(Body::link(index));
    }

    DyLink* link(const std::string& name) const {
        return static_cast<DyLink*>(Body::link(name));
    }

    DyLink* rootLink() const {
        return static_cast<DyLink*>(Body::rootLink());
    }

    void calcSpatialForwardKinematics();
};

typedef ref_ptr<DyBody> DyBodyPtr;

}

#endif
