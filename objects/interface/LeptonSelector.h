#ifndef LeptonSelector_H
#define LeptonSelector_H

class Lepton;

class LeptonSelector {
    friend class Lepton;

    public:
        bool isLoose() const;
        bool isFO() const;
        bool isTight() const;

        virtual double coneCorrection() const = 0;

        virtual ~LeptonSelector(){}

    protected:

	bool isFOttH() const;
	bool isTightttH() const;
	bool isFOtZq() const;
	bool isTighttZq() const;

        virtual bool isLooseBase() const = 0;
        virtual bool isLoose2016() const = 0;
        virtual bool isLoose2017() const = 0;
        virtual bool isLoose2018() const = 0;

        virtual bool isFOBase() const = 0;
        virtual bool isFO2016() const = 0;
        virtual bool isFO2017() const = 0;
        virtual bool isFO2018() const = 0;
        virtual bool isFOBasetZq() const = 0;
        virtual bool isFO2016tZq() const = 0;
        virtual bool isFO2017tZq() const = 0;
	virtual bool isFO2018tZq() const = 0;

        virtual bool isTightBase() const = 0;
        virtual bool isTight2016() const = 0;
        virtual bool isTight2017() const = 0;
        virtual bool isTight2018() const = 0;
        virtual bool isTightBasetZq() const = 0;
        virtual bool isTight2016tZq() const = 0;
        virtual bool isTight2017tZq() const = 0;
	virtual bool isTight2018tZq() const = 0;

        virtual bool is2016() const = 0;
        virtual bool is2017() const = 0;

    private:
        virtual LeptonSelector* clone() const & = 0;
        virtual LeptonSelector* clone() && = 0;
};
#endif
