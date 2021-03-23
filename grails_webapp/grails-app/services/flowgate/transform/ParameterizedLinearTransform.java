package flowgate.transform;

public class ParameterizedLinearTransform extends Transform {

    double T;
    double A;
    /**
     * The cached inverse of (T + A).
     */
    double inverseSum;

    ParameterizedLinearTransform(double t, double a) {
        this.T = t;
        this.A = a;
        this.inverseSum = (t == 0.0 && a == 0.0) ? 0.0 : (1.0 / (t + a));
    }

    public double getT() {
        return T;
    }

    public void setT(double t) {
        T = t;
    }

    public double getA() {
        return A;
    }

    public void setA(double a) {
        A = a;
    }

    double transformInternal(double value) {
        return (value + this.A) * this.inverseSum;
    }

    double[] transformInternal(int size, double[] array) {
        double is = this.inverseSum;

        for (int i = 0; i < size; ++i)
            array[i] = (this.A + array[i]) * is;

        return array;
    }

    float[] transformInternal(int size, float[] array) {
        double is = this.inverseSum;

        for (int i = 0; i < size; ++i)
            array[i] = (float) ((this.A + array[i]) * is);

        return array;
    }

    @Override
    public String toString() {
        return "ParameterizedLinearTransform{" +
                "T=" + T +
                ", A=" + A +
                '}';
    }
}
