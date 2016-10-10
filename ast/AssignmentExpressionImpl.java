package ast;

import lombok.Value;

import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class AssignmentExpressionImpl implements AssignmentExpression, Assigning {

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
            return null;
        }

        public String getToken() {
            return token;
        }
    }

    private UnaryExpression unaryExpression;
    private AssignmentOperator assignmentOperator;
    private AssignmentExpression assignmentExpression;

    @Override
    public String toCode() {
        return unaryExpression.toCode() + assignmentOperator.token + assignmentExpression.toCode();
    }

    @Override
    public Set<String> getLValues() {
        return unaryExpression.getLValues();
    }

    @Override
    public Set<String> getRightVariables() {
        return assignmentExpression.getVariables();
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        variables.addAll(unaryExpression.getVariables());
        variables.addAll(assignmentExpression.getVariables());
        return variables;
    }
}
