package ast;

import java.util.Comparator;

/**
 * Created by ooee on 10/25/16.
 */
public class ExpressionComparator implements Comparator<Expression> {
    @Override
    public int compare(Expression e1, Expression e2) {
        if (e1.getClass().equals(e2.getClass())) {
            return Integer.compare(e1.pointValue(), e2.pointValue());
        }
        if (e1.getClass().isInstance(e2)) {
            return -1;
        }
        if (e2.getClass().isInstance(e1)) {
            return 1;
        }
        throw new UnsupportedOperationException("What happened here?");
    }
}
