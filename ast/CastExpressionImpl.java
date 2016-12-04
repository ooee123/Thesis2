package ast;

import ast.type.Type;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class CastExpressionImpl implements CastExpression {
    private Type type;
    private CastExpression castExpression;

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return castExpression.getGuaranteedChangedVariables();
    }

    public String toCode() {
        return "(" + type.toCode() + ")" + castExpression.toCode();
    }

    @Override
    public Set<String> getDependentVariables() {
        return castExpression.getDependentVariables();
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return castExpression.getInvocations();
    }

    @Override
    public Set<String> getVariables() {
        return castExpression.getVariables();
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(castExpression);
    }
}
