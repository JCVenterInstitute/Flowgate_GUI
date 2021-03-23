package flowgate.transform;

public class ParameterizedLogarithmicTransform extends Transform {

    double T;
    double M;
    double inverseT;
    double inverseM;

    ParameterizedLogarithmicTransform(double t, double m) {
        this.T = t;
        this.M = M;
        this.inverseT = (t == 0.0) ? 0.0 : (1.0 / t);
        this.inverseM = (m == 0.0) ? 0.0 : (1.0 / m);
    }

    public double getT() {
        return T;
    }

    public void setT(double t) {
        T = t;
    }

    public double getM() {
        return M;
    }

    public void setM(double m) {
        M = m;
    }

    double transformInternal(double value) {
        return this.inverseM * Math.log10(value * this.inverseT) + 1.0;
    }

    double[] transformInternal(int size, double[] array) {
        double it = this.inverseT;
        double im = this.inverseM;

        for (int i = 0; i < size; ++i)
            array[i] = im * Math.log10(array[i] * it) + 1.0;

        return array;
    }

    float[] transformInternal(int size, float[] array) {
        double it = this.inverseT;
        double im = this.inverseM;

        for (int i = 0; i < size; ++i)
            array[i] = (float) (im * Math.log10(array[i] * it) + 1.0);

        return array;
    }

    @Override
    public String toString() {
        return "ParameterizedLinearTransform{" +
                "T=" + T +
                ", M=" + M +
                '}';
    }
}
