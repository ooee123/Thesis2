package visitor;

import ast.*;

import java.util.*;

/**
 * Created by ooee on 10/21/16.
 */
public class FunctionArgumentOrderVisitor {
    private Map<String, Collection<PostfixExpressionInvocationImpl>> invocations;
    private Map<String, Function> functionDefinition;

    public FunctionArgumentOrderVisitor() {
        this.invocations = new HashMap<>();
        this.functionDefinition = new HashMap<>();
    }

    public void visit(Program p) {

        Collection<PostfixExpressionInvocationImpl> invocations = new ArrayList<>();

        Visitor<Expression> visitor = new Visitor<Expression>() {
            @Override
            public void visit(Expression expression) {
                invocations.addAll(expression.getInvocations());
            }
        };

        Visitor<Expression> visitor2 = new Visitor<Expression>() {
            @Override
            public void visit(Expression expression) {
                if (expression instanceof PostfixExpressionInvocationImpl) {
                    invocations.add(((PostfixExpressionInvocationImpl) expression));
                }
                expression.visitNestedExpressions(this);
            }
        };

        Visitor<Expression> functionNameDigger = new Visitor<Expression>() {
            @Override
            public void visit(Expression expression) {
                invocations.addAll(expression.getInvocations());
            }
        };


        for (Function function : p.getFunction()) {
            functionDefinition.put(function.getIdentifier(), function);
            function.getCompoundStatement().visitAllExpressions(visitor);
        }
        for (String functionName : functionDefinition.keySet()) {
            Collection<PostfixExpressionInvocationImpl> calls = new ArrayList<>();
            for (PostfixExpressionInvocationImpl invocation : invocations) {
                if (invocation.getPostfixExpression() instanceof PrimaryExpressionIdentifier) {
                    String functionID = ((PrimaryExpressionIdentifier) invocation.getPostfixExpression()).getIdentifier();
                    if (functionID.equals(functionName)) {
                        calls.add(invocation);
                    }
                }
            }
            this.invocations.put(functionName, calls);
        }
        System.out.println("Invocations");
        for (Map.Entry<String, Collection<PostfixExpressionInvocationImpl>> stringCollectionEntry : this.invocations.entrySet()) {
            System.out.println(stringCollectionEntry);
        }
    }



    private void reorderParameters(String functionName, List<Integer> newOrder) {
        Function function = functionDefinition.get(functionName);
        List<Parameter> parameters = function.getParameterList().getParameters();
        List<Parameter> reorderedParameters = new ArrayList<>();
        for (Integer integer : newOrder) {
            reorderedParameters.add(parameters.get(integer));
        }
        function.setParameterList(new ParameterList(reorderedParameters));

        for (PostfixExpressionInvocationImpl postfixExpressionInvocation : invocations.get(functionName)) {
            List<AssignmentExpression> arguments = postfixExpressionInvocation.getArguments();
            List<AssignmentExpression> newArguments = new ArrayList<>();
            for (Integer integer : newOrder) {
                newArguments.add(arguments.get(integer));
            }
            postfixExpressionInvocation.setArguments(newArguments);
        }
    }
}
