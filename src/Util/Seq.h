/**
   @file
   @author Shin'ichiro Nakaoka
*/

#ifndef CNOID_UTIL_SEQ_H
#define CNOID_UTIL_SEQ_H

#include "AbstractSeq.h"
#include <memory>
#include <vector>
#include "exportdecl.h"

namespace cnoid {

template <typename ElementType> class Seq : public AbstractSeq
{
    typedef Seq<ElementType> SeqType;
        
public:
    typedef ElementType value_type;
    typedef std::shared_ptr<SeqType> Ptr;
        
    Seq(const char* seqType, int nFrames = 0.0)
        : AbstractSeq(seqType),
          container(nFrames) {
        frameRate_ = defaultFrameRate();
    }
        
    Seq(const SeqType& org)
        : AbstractSeq(org),
          container(org.container) {
        frameRate_ = org.frameRate_;
    }

    SeqType& operator=(const SeqType& rhs) {
        if(this != &rhs){
            AbstractSeq::operator=(rhs);
            container = rhs.container;
            frameRate_ = rhs.frameRate_;
        }
        return *this;
    }

    virtual AbstractSeq& operator=(const AbstractSeq& rhs) override {
        const SeqType* rhsSeq = dynamic_cast<const SeqType*>(&rhs);
        if(rhsSeq){
            return operator=(*rhsSeq);
        } else {
            return AbstractSeq::operator=(rhs);
        }
    }

    virtual AbstractSeqPtr cloneSeq() const override {
        return std::make_shared<SeqType>(*this);
    }
        
    virtual ~Seq() { }
        
    virtual double getFrameRate() const override {
        return frameRate_;
    }

    double frameRate() const {
        return frameRate_;
    }

    virtual void setFrameRate(double frameRate) override {
        frameRate_ = frameRate;
    }

    virtual int getNumFrames() const override {
        return container.size();
    }

    int numFrames() const {
        return container.size();
    }

    virtual void setNumFrames(int n, bool fillNewElements = false) override {
        if(fillNewElements){
            const int nold = numFrames();
            if(n > nold && nold > 0){
                container.resize(n, container[nold - 1]);
            } else {
                container.resize(n, defaultValue());
            }
        } else {
            container.resize(n);
        }
    }

    bool empty() const {
        return container.empty();
    }

    int frameOfTime(double time) const {
        return (int)(time * frameRate_);
    }

    double timeOfFrame(int frame) const {
        return (frame / frameRate_);
    }

    ElementType& operator[](int frameIndex) {
        return container[frameIndex];
    }

    const ElementType& operator[](int frameIndex) const {
        return container[frameIndex];
    }

    ElementType& at(int frameIndex) {
        return container[frameIndex];
    }

    const ElementType& at(int frameIndex) const {
        return container[frameIndex];
    }

    int clampFrameIndex(int frameIndex, bool& out_isValidRange){
        if(frameIndex < 0){
            frameIndex = 0;
            out_isValidRange = false;
        } else if(frameIndex >= numFrames()){
            frameIndex = numFrames() - 1;
            out_isValidRange = false;
        } else {
            out_isValidRange = true;
        }
        return frameIndex;
    }

protected:

    std::vector<ElementType> container;
    double frameRate_;

    virtual ElementType defaultValue() const { return ElementType(); }
};

}

#endif
