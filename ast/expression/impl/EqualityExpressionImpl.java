package ast.expression.impl;

import ast.expression.EqualityExpression;
import ast.expression.Expression;
import ast.expression.RelationalExpression;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class EqualityExpressionImpl implements EqualityExpression {
    public enum EqualityOperator {
        EQUALS("=="),
        NOT_EQUALS("!=");

        public final String token;

        EqualityOperator(String token) {
            this.token = token;
        }

        public static EqualityOperator toEqualityOperator(String token) {
            EqualityOperator[] equalityOperators = EqualityOperator.values();
            for (EqualityOperator equalityOperator : equalityOperators) {
                if (equalityOperator.token.equals(token)) {
                    return equalityOperator;
                }
            }
            throw new IllegalArgumentException("Token " + token + " not recognized");
        }
    }

    private EqualityExpression equalityExpression;
    private EqualityOperator equalityOperator;
    private RelationalExpression relationalExpression;

    @Override
    public String toCode() {
        return equalityExpression.toCode() + " " + equalityOperator.token + " " + relationalExpression.toCode();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return multiGetSet(exp -> exp.getGuaranteedChangedVariables(), equalityExpression, relationalExpression);
    }

    @Override
    public Set<String> getVariables() {
        return multiGetSet(exp -> exp.getVariables(), equalityExpression, relationalExpression);
        /*
        Set<String> variables = new HashSet<>();
        variables.addAll(equalityExpression.getVariables());
        variables.addAll(relationalExpression.getVariables());
        return variables;
        */
    }

    @Override
    public Set<String> getDependentVariables() {
        return multiGetSet(exp -> exp.getDependentVariables(), equalityExpression, relationalExpression);
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return multiGetIdentitySet(exp -> exp.getInvocations(), equalityExpression, relationalExpression);
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(equalityExpression);
        visitor.visit(relationalExpression);
    }
}
