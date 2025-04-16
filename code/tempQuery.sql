SELECT 
    c.customer_id, 
    c.first_name, 
    c.last_name, 
    o.order_id, 
    o.order_date, 
    p.product_name, 
    oi.quantity, 
    oi.unit_price
FROM 
    customers c
JOIN 
    orders o ON c.customer_id = o.customer_id
JOIN 
    order_items oi ON o.order_id = oi.order_id
JOIN 
    products p ON oi.product_id = p.product_id
WHERE 
    o.order_date BETWEEN '2023-01-01' AND '2023-12-31'
    AND p.category_id IN (1, 2, 3)
ORDER BY 
    c.last_name, o.order_date DESC;;