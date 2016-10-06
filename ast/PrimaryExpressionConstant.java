package ast;

import lombok.Value;

/**
 * Created by kly04 on 9/26/16.
 */
@Value
public class PrimaryExpressionConstant implements PrimaryExpression {
    private Object value;

    public PrimaryExpressionConstant(long l) {
        value = l;
    }

    public PrimaryExpressionConstant(double d) {
        value = d;
    }

    public PrimaryExpressionConstant(char c) {
        value = c;
    }

    public PrimaryExpressionConstant(String s) {
        value = s;
    }

    @Override
    public String toCode() {
        return value.toString();
    }
}