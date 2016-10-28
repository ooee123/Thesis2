package ast;

import lombok.NonNull;
import lombok.Value;
import visitor.Visitor;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class AssignmentExpressionImpl implements AssignmentExpression {

    public enum AssignmentOperator {
        ASSIGNMENT("="),
        MULT_ASSIGNMENT("*="),
        DIV_ASSIGNMENT("/="),
        MOD_ASSIGNMENT("%="),
        ADD_ASSIGNMENT("+="),
        SUB_ASSIGNMENT("-="),
        LEFT_SHIFT_ASSIGNMENT("<<="),
        RIGHT_SHIFT_ASSIGNMENT(">>="),
        BIT_AND_ASSIGNMENT("&="),
        BIT_XOR_ASSIGNMENT("^="),
        BIT_OR_ASSIGNMENT("|=");

        private String token;

        AssignmentOperator(String operator) {
            token = operator;
        }

        public static AssignmentOperator toAssignmentOperator(String token) {
            AssignmentOperator[] assignmentOperators = AssignmentOperator.values();
            for (AssignmentOperator assignmentOperator : assignmentOperators) {
                if (assignmentOperator.getToken().equals(token)) {
                    return assignmentOperator;
                }
            }
            throw new IllegalArgumentException("Token " + token + " not recognized");
        }

        public String getToken() {
            return token;
        }
    }

    @NonNull private UnaryExpression unaryExpression;
    @NonNull private AssignmentOperator assignmentOperator;
    @NonNull private AssignmentExpression assignmentExpression;

    @Override
    public String toCode() {
        return unaryExpression.toCode() + " " + assignmentOperator.token + " " + assignmentExpression.toCode();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        Set<String> lValues = new HashSet<>();
        lValues.addAll(assignmentExpression.getGuaranteedChangedVariables());
        lValues.addAll(unaryExpression.getVariables());
        return lValues;
    }

    @Override
    public Set<String> getDependentVariables() {
        Set<String> dependentVariables = new HashSet<>();
        dependentVariables.addAll(assignmentExpression.getDependentVariables());
        if (unaryExpression instanceof UnaryExpressionUnaryOperatorImpl) {
            UnaryExpressionUnaryOperatorImpl unaryExpression = (UnaryExpressionUnaryOperatorImpl) this.unaryExpression;
            if (unaryExpression.getUnaryOperator().equals(UnaryExpressionUnaryOperatorImpl.UnaryOperator.DEREFERENCE)) {
                // Is a dereference operator
                // We'll do a grand sweeping one. All involved variables on left side are dependent
                dependentVariables.addAll(unaryExpression.getVariables());
            }
        }
        if (unaryExpression instanceof PostfixExpressionArrayAccessImpl) {
            PostfixExpressionArrayAccessImpl unaryExpression = (PostfixExpressionArrayAccessImpl) this.unaryExpression;
            // Array access
            unaryExpression.getDependentVariables();
        }
        if (!assignmentOperator.equals(AssignmentOperator.ASSIGNMENT)) {
            dependentVariables.addAll(unaryExpression.getVariables());
        }
        return dependentVariables;

    }

    @Override
    public List<PostfixExpressionInvocationImpl> getInvocations() {
        List<PostfixExpressionInvocationImpl> invocations = new ArrayList<>();
        invocations.addAll(unaryExpression.getInvocations());
        invocations.addAll(assignmentExpression.getInvocations());
        return invocations;
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        variables.addAll(unaryExpression.getVariables());
        variables.addAll(assignmentExpression.getVariables());
        return variables;
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(unaryExpression);
        visitor.visit(assignmentExpression);
    }
}
