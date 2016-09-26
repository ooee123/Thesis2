package ast;

import lombok.Value;

/**
 * Created by kly04 on 9/26/16.
 */
@Value
public class PrimaryExpressionConstant implements PrimaryExpression {
    public PrimaryExpressionConstant(long l) {

    }

    public PrimaryExpressionConstant(double d) {

    }

    public PrimaryExpressionConstant(char c) {

    }

    public PrimaryExpressionConstant(String s) {

    }
}
