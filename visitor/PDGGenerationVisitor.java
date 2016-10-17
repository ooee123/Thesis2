package visitor;

import ast.*;
import ast.Function;
import ast.type.PointerType;
import lombok.AllArgsConstructor;
import lombok.Value;
import pdg.PDGNode;

import java.util.*;
import java.util.stream.Collectors;

@Value
public class PDGGenerationVisitor {

    public Collection<PDGNode> getPDGNodes() {
        return allNodes.values();
    }

    private Map<BlockItem, PDGNode> allNodes = new IdentityHashMap<>();

    public void link(BlockItem from, BlockItem to) {
        if (!allNodes.containsKey(from)) {
            allNodes.put(from, new PDGNode(from, from.isCritical()));
        }
        if (!allNodes.containsKey(to)) {
            allNodes.put(to, new PDGNode(to, to.isCritical()));
        }
        PDGNode fromNode = allNodes.get(from), toNode = allNodes.get(to);
        fromNode.getIsADependencyFor().add(toNode);
        toNode.getDependsOn().add(fromNode);
    }

    public void printAdjacencyMatrix() {
        int i = 0;
        String label;
        System.out.println("Adjacency Matrix");
        Map<PDGNode, String> labels = new IdentityHashMap<>();
        for (PDGNode pdgNode : allNodes.values()) {
            label = String.valueOf(Character.valueOf((char) ('A' + i)));
            labels.put(pdgNode, label);
            System.out.println(label + " := " + pdgNode.getBlockItem().toCode());
            i++;
        }

        for (PDGNode pdgNode : allNodes.values()) {
            String source = labels.get(pdgNode);
            System.out.print(source + ": ");
            List<String> destLabels = pdgNode.getIsADependencyFor().stream().map(node -> labels.get(node)).collect(Collectors.toList());
            System.out.println(String.join(", ", destLabels));
        }
        System.out.println("End Matrix");
    }


    @Value
    @AllArgsConstructor
    class Dependencies {
        private Set<String> dependentVariables;
        private Set<String> changedVariables;

        Dependencies() {
            dependentVariables = Collections.emptySet();
            changedVariables = Collections.emptySet();
        }

        Dependencies add(Dependencies d) {
            Set<String> otherDependentVariables = new HashSet<>(d.dependentVariables);
            otherDependentVariables.removeAll(this.changedVariables);
            this.dependentVariables.addAll(otherDependentVariables);
            this.changedVariables.addAll(d.changedVariables);
            return this;
        }
    }

    private void nullableExpression(Expression expression, Dependencies dependencies) {
        if (expression != null) {
            Set<String> rightVariables = expression.getDependentVariables();
            rightVariables.removeAll(dependencies.changedVariables);
            dependencies.dependentVariables.addAll(rightVariables);
            dependencies.changedVariables.addAll(expression.getChangedVariables());
        }
    }

    public void visit(Program p) {
        for (Function function : p.getFunction()) {
            visit(function);
        }
        printAdjacencyMatrix();
    }

    private Dependencies visit(Function function) {
        Dependencies functionDependencies = visit(function.getCompoundStatement());
        for (Parameter parameter : function.getParameterList().getParameters()) {
            // If the type is a pointer, mark it as changed.
            //functionDependencies.


            if (parameter.getType() instanceof PointerType && functionDependencies.changedVariables.contains(parameter.getFormalParameterName())) {
                // Pointer value changed

            }
        }
        visit(function.getCompoundStatement());
        //functionDependencies.getChangedVariables()
        return null;
    }

    private Dependencies visit(Statement statement) {
        if (statement instanceof CompoundStatement) {
            return visit(((CompoundStatement) statement));
        } else if (statement instanceof ExpressionStatement) {
            return visit(((ExpressionStatement) statement));
        } else if (statement instanceof SelectionStatementIf) {
            return visit(((SelectionStatementIf) statement));
        } else if (statement instanceof IterationStatementFor) {
            return visit(((IterationStatementFor) statement));
        } else if (statement instanceof IterationStatementDeclareFor) {
            return visit(((IterationStatementDeclareFor) statement));
        } else if (statement instanceof IterationStatementWhile) {
            return visit(((IterationStatementWhile) statement));
        } else if (statement instanceof IterationStatementDoWhile) {
            return visit(((IterationStatementDoWhile) statement));
        } else {
            return new Dependencies(statement.getDependantVariables(), statement.getChangedVariables());
        }
    }

    private Dependencies visit(IterationStatementFor statement) {
        Dependencies dependencies = new Dependencies();
        nullableExpression(statement.getInitial(), dependencies);
        nullableExpression(statement.getCondition(), dependencies);
        Dependencies bodyDependencies = visit(statement.getStatement());
        bodyDependencies.dependentVariables.removeAll(dependencies.changedVariables);
        dependencies.dependentVariables.addAll(bodyDependencies.dependentVariables);
        dependencies.changedVariables.addAll(bodyDependencies.changedVariables);
        nullableExpression(statement.getIteration(), dependencies);
        return dependencies;
    }

    private Dependencies visit(IterationStatementDeclareFor statement) {
        Set<String> declaredVariables = new HashSet<>();
        Dependencies dependencies = new Dependencies();
        for (Declaration.DeclaredVariable declaredVariable : statement.getDeclaration().getDeclaredVariables()) {
            nullableExpression(declaredVariable.getInitializer(), dependencies);
            declaredVariables.add(declaredVariable.getIdentifier());
        }
        nullableExpression(statement.getCondition(), dependencies);
        Dependencies bodyDependencies = visit(statement.getStatement());
        bodyDependencies.dependentVariables.removeAll(dependencies.changedVariables);
        dependencies.dependentVariables.addAll(bodyDependencies.dependentVariables);
        bodyDependencies.changedVariables.removeAll(declaredVariables);
        dependencies.changedVariables.addAll(bodyDependencies.changedVariables);
        nullableExpression(statement.getIteration(), dependencies);
        return dependencies;
    }

    private Dependencies visit(IterationStatementWhile statement) {
        Dependencies dependencies = new Dependencies();
        nullableExpression(statement.getCondition(), dependencies);
        Dependencies bodyDependencies = visit(statement.getStatement());
        dependencies.add(bodyDependencies);
        return dependencies;
    }

    private Dependencies visit(IterationStatementDoWhile statement) {
        Dependencies dependencies = new Dependencies();
        nullableExpression(statement.getCondition(), dependencies);
        Dependencies bodyDependencies = visit(statement.getStatement());
        dependencies.add(bodyDependencies);
        return dependencies;
    }

    private Dependencies visit(ExpressionStatement statement) {
        return visit(statement.getExpression());
    }

    private Dependencies visit(Expression expression) {
        Set<String> dependentVariables = expression.getDependentVariables();
        Set<String> changedVariables = expression.getChangedVariables();
        return new Dependencies(dependentVariables, changedVariables);
    }

    private Dependencies visit(CompoundStatement statement) {
        Map<String, BlockItem> lastAssigned = new HashMap<>();
        Set<String> incomingValues = new HashSet<>();
        for (BlockItem blockItem : statement.getBlockItems()) {
            if (blockItem instanceof Statement) {
                System.out.println(blockItem.toCode());
                Dependencies dependencies;
                if (blockItem instanceof JumpStatementStrict) {
                    dependencies = new Dependencies(lastAssigned.keySet(), Collections.emptySet());
                } else {
                    dependencies = visit(((Statement) blockItem));
                }
                System.out.println(dependencies);
                for (String usedVariable : dependencies.getDependentVariables()) {
                    if (!lastAssigned.containsKey(usedVariable)) {
                        incomingValues.add(usedVariable);
                    } else {
                        BlockItem from = lastAssigned.get(usedVariable);
                        BlockItem to = blockItem;
                        link(from, to);
                    }
                }
                for (String changedVariables : dependencies.getChangedVariables()) {
                    lastAssigned.put(changedVariables, blockItem);
                }
                if (blockItem instanceof JumpStatementStrict) {
                    return new Dependencies(incomingValues, lastAssigned.keySet());
                }
            } else if (blockItem instanceof Declaration) {
                Declaration declaration = (Declaration) blockItem;
                for (Declaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
                    if (declaredVariable.getInitializer() == null) {
                        lastAssigned.put(declaredVariable.getIdentifier(), declaration);
                    } else {
                        lastAssigned.put(declaredVariable.getIdentifier(), declaration);
                        Expression initializer = declaredVariable.getInitializer();
                        Set<String> rightVariables = initializer.getDependentVariables();
                        rightVariables.removeAll(lastAssigned.keySet());
                        incomingValues.addAll(rightVariables);
                        for (String lvalue : initializer.getChangedVariables()) {
                            lastAssigned.put(lvalue, declaration);
                        }
                    }
                }
            }
        }
        return new Dependencies(incomingValues, lastAssigned.keySet());
    }

    private Dependencies visit(SelectionStatementIf statement) {
        Set<String> dependentVariables = new HashSet<>();
        Set<String> assignedVariables = new HashSet<>();
        Dependencies conditionDependencies = visit(statement.getCondition());
        dependentVariables.addAll(conditionDependencies.getDependentVariables());
        assignedVariables.addAll(conditionDependencies.getChangedVariables());

        Statement thenStatement = statement.getThenStatement();
        Dependencies thenDependencies = visit(thenStatement);
        thenDependencies.dependentVariables.removeAll(assignedVariables);
        dependentVariables.addAll(thenDependencies.dependentVariables);


        if (statement.getElseStatement() != null) {
            Dependencies elseDependencies;
            elseDependencies = visit(statement.getElseStatement());
            elseDependencies.dependentVariables.removeAll(assignedVariables);
            dependentVariables.addAll(elseDependencies.dependentVariables);
            assignedVariables.addAll(elseDependencies.changedVariables);
        }
        assignedVariables.addAll(thenDependencies.changedVariables);
        return new Dependencies(dependentVariables, assignedVariables);
    }

    private void printLastAssigned(Map<String, BlockItem> map) {
        for (Map.Entry<String, BlockItem> stringBlockItemEntry : map.entrySet()) {
            System.out.println(stringBlockItemEntry.getKey() + ": " + stringBlockItemEntry.getValue().toCode());
        }
    }
}
